# Python program for insertion and deletion in Circular Queue
class Queue:
    def __init__(self, c):
        self.arr = [0] * c
        self.capacity = c
        self.size = 0
        self.front = 0

    # Get the front element
    def getFront(self):
        # Queue is empty
        if self.size == 0:
            return -1
        return self.arr[self.front]

    # Get the rear element
    def getRear(self):
        # Queue is empty
        if self.size == 0:
            return -1
        rear = (self.front + self.size - 1) % self.capacity
        return self.arr[rear]

    # Insert an element at the rear
    def enqueue(self, x):
        # Queue is full
        if self.size == self.capacity:
            return
        rear = (self.front + self.size) % self.capacity
        self.arr[rear] = x
        self.size += 1

    # Remove an element from the front
    def dequeue(self):
        # Queue is empty
        if self.size == 0:
            return -1
        res = self.arr[self.front]
        self.front = (self.front + 1) % self.capacity
        self.size -= 1
        return res

if __name__ == '__main__':
    q = Queue(4)
    q.enqueue(10)
    print(q.getFront(), q.getRear())
    q.enqueue(20)
    print(q.getFront(), q.getRear())
    q.enqueue(30)
    print(q.getFront(), q.getRear())
    q.enqueue(40)
    print(q.getFront(), q.getRear())
    q.dequeue()
    print(q.getFront(), q.getRear())
    q.dequeue()
    print(q.getFront(), q.getRear())
    q.enqueue(50)
    print(q.getFront(), q.getRear())
