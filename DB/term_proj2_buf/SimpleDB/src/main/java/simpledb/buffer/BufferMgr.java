package simpledb.buffer;

import simpledb.file.*;
import simpledb.log.LogMgr;

import java.util.HashMap;
import java.util.Map;
import java.util.Iterator;
import java.util.LinkedList;

/**
 * Manages the pinning and unpinning of buffers to blocks.
 * @author Edward Sciore
 *
 */
public class BufferMgr {
   private int numAvailable;   /* the number of available (unpinned) buffer slots */
   private static final long MAX_TIME = 10000; /* 10 seconds */
   private Map<BlockId, Buffer> bufferMap;
   private LinkedList<Buffer> UnpinnedList;
   /**
    * Constructor:  Creates a buffer manager having the specified 
    * number of buffer slots.
    * This constructor depends on a {@link FileMgr} and
    * {@link simpledb.log.LogMgr LogMgr} object.
    * @param numbuffs the number of buffer slots to allocate
    */
   public BufferMgr(FileMgr fm, LogMgr lm, int numbuffs) {
      numAvailable = numbuffs;
      bufferMap = new HashMap<>(numbuffs);
      UnpinnedList = new LinkedList<>();
      for (int i = 0; i < numbuffs; i++) {
         // BlockId blk = new BlockId(null, i);
         Buffer buff = new Buffer(fm, lm, i);
         // bufferMap.put(blk, buff);
         UnpinnedList.add(buff);
      }
   }
   
   /**
    * Returns the number of available (i.e. unpinned) buffers.
    * @return the number of available buffers
    */
   public synchronized int available() {
      return numAvailable;
   }
   

   /**
    * Flushes the dirty buffers modified by the specified transaction.
    * @param txnum the transaction's id number
    */
   public synchronized void flushAll(int txnum) {
      for (Buffer buff : bufferMap.values())
         if (buff.modifyingTx() == txnum)
            buff.flush();
   }
   
   /**
    * Unpins the specified data buffer. If its pin count
    * goes to zero, then notify any waiting threads.
    * @param buff the buffer to be unpinned
    */
   public synchronized void unpin(Buffer buff) {
      buff.unpin();
      if (!buff.isPinned()) {
         BlockId blockId = buff.block();
         if (blockId != null) {
            for (Buffer b : UnpinnedList) {
               if (b.block().equals(blockId)) {
                  UnpinnedList.remove(b);
                  break;
               }
            }
         }
         UnpinnedList.add(buff);
         numAvailable++;
         notifyAll();
      }
   } 
   
   /**
    * Pins a buffer to the specified block, potentially
    * waiting until a buffer becomes available.
    * If no buffer becomes available within a fixed 
    * time period, then a {@link BufferAbortException} is thrown.
    * @param blk a reference to a disk block
    * @return the buffer pinned to that block
    */
   public synchronized Buffer pin(BlockId blk) {
      try {
         long timestamp = System.currentTimeMillis();
         Buffer buff = tryToPin(blk);
         while (buff == null && !waitingTooLong(timestamp)) {
            wait(MAX_TIME);
            buff = tryToPin(blk);
         }
         if (buff == null)
            throw new BufferAbortException();
         return buff;
      }
      catch(InterruptedException e) {
         throw new BufferAbortException();
      }
   }  
   
   /**
    * Returns true if starttime is older than 10 seconds
    * @param starttime timestamp 
    * @return true if waited for more than 10 seconds
    */
   private boolean waitingTooLong(long starttime) {
      return System.currentTimeMillis() - starttime > MAX_TIME;
   }
   
   /**
    * Tries to pin a buffer to the specified block. 
    * If there is already a buffer assigned to that block
    * then that buffer is used;  
    * otherwise, an unpinned buffer from the pool is chosen.
    * Returns a null value if there are no available buffers.
    * @param blk a reference to a disk block
    * @return the pinned buffer
    */
   private Buffer tryToPin(BlockId blk) {
      Buffer buff = findExistingBuffer(blk);
      if (buff == null) {
         buff = chooseUnpinnedBuffer();
         if (buff == null)
            return null;
         bufferMap.remove(buff.block());
         buff.assignToBlock(blk);
         bufferMap.put(blk, buff);
      }
      if (!buff.isPinned())
         numAvailable--;
      
      buff.pin();
      removeByValue(UnpinnedList, buff);

      return buff;
   }

   private void removeByValue(LinkedList<Buffer> list, Buffer value) {
      Iterator<Buffer> iterator = list.iterator();
      while (iterator.hasNext()) {
         Buffer buffer = iterator.next();
         if (buffer == value) {
            iterator.remove();
            break;
         }
      }
   }

   /**
    * Find and return a buffer assigned to the specified block. 
    * @param blk a reference to a disk block
    * @return the found buffer       
    */
   private Buffer findExistingBuffer(BlockId blk) {
      return bufferMap.get(blk);
   }
   
   /**
    * Find and return an unpinned buffer     . 
    * @return the unpinned buffer       
    */
   private Buffer chooseUnpinnedBuffer() {
      // for(Buffer buff : bufferMap.values()){
      //    if (!buff.isPinned()){       
      //       return buff;
      //    }
      // }
      if(!UnpinnedList.isEmpty()){
         Buffer buff = UnpinnedList.removeFirst();
         numAvailable++;
         return buff;
      }
      return null;
   }
   

   public void printStatus(){
      System.out.println("Allocated Buffers:");

      if(bufferMap.isEmpty()){
         System.out.println("empty hash map");
      }

      for(Map.Entry<BlockId, Buffer> entry : bufferMap.entrySet()) {
         BlockId blockId = entry.getKey();
         Buffer buff = entry.getValue();
      
         String blkString = blockId.toString();
         String pinStatus = buff.isPinned() ? "pinned" : "unpinned";
      
         System.out.println("Buffer " + buff.getId() + ": " + blkString + " " + pinStatus);
      }

      System.out.print("Unpinned Buffers in LRU order:");
      for(Buffer buff : UnpinnedList) {
         System.out.print(" " + buff.getId());
      }
      System.out.println("");
   }
}
