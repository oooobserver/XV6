// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define NBUCKET 10

struct {
  struct spinlock lock;
  struct buf buf[NBUF];
} bcache;

struct bucket {
  struct spinlock lock;
  struct buf head;
}hashtable[NBUCKET];

int hash_function(uint blockno){
  return blockno % NBUCKET;
}

void binit(void) {
  struct buf *b;

  initlock(&bcache.lock, "bcache");

  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    initsleeplock(&b->lock, "buffer");
  }

  b = bcache.buf;
  for (int i = 0; i < NBUCKET; i++) {
    initlock(&hashtable[i].lock, "bcache_bucket");
    for (int j = 0; j < NBUF / NBUCKET; j++) {
      b->next = hashtable[i].head.next;
      hashtable[i].head.next = b;
      b++;
    }
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf* bget(uint dev, uint blockno) {
  struct buf *b;

  int index = hash_function(blockno);
  acquire(&hashtable[index].lock);

  // Is the block already cached?
  for(b = hashtable[index].head.next; b != 0; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&hashtable[index].lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  struct buf* target = 0;

  for(b = hashtable[index].head.next; b != 0; b = b->next){
    if(b->refcnt == 0) {
      target = b;
    }
  }
  if(target) {
    goto find;
  }

  // Try to find in other bucket.
  acquire(&bcache.lock);

  again:
  for(b = bcache.buf; b < bcache.buf + NBUF; b++) {
    if(b->refcnt == 0) {
      target = b;
    }
  }
  if (target) {
    // remove from old bucket
    int rindex = hash_function(target->blockno);
    acquire(&hashtable[rindex].lock);

    // Edge case
    if(target->refcnt != 0) {
      release(&hashtable[rindex].lock);
      goto again;
    }

    struct buf *pre = &hashtable[rindex].head;
    struct buf *p = hashtable[rindex].head.next;
    while (p != target) {
      pre = pre->next;
      p = p->next;
    }
    pre->next = p->next;
    release(&hashtable[rindex].lock);

    target->next = hashtable[index].head.next;
    hashtable[index].head.next = target;
    release(&bcache.lock);
    goto find;
  }
  else {
    panic("bget: no buffers");
  }

  find:
    target->dev = dev;
    target->blockno = blockno;
    target->valid = 0;
    target->refcnt = 1;
    release(&hashtable[index].lock);
    acquiresleep(&target->lock);
    return target;
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void brelse(struct buf *b) {
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);
  bunpin(b);
}

void bpin(struct buf *b) {
  int index = hash_function(b->blockno);
  acquire(&hashtable[index].lock);
  b->refcnt++;
  release(&hashtable[index].lock);
}

void bunpin(struct buf *b) {
  int index = hash_function(b->blockno);
  acquire(&hashtable[index].lock);
  b->refcnt--;
  release(&hashtable[index].lock);
}