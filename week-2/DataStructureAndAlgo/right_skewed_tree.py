# Code from https://www.geeksforgeeks.org/dsa/skewed-binary-tree/

# Python3 implementation of the above approach 

# A Tree node 
class Node: 
    
    def __init__(self, key): 
        
        self.left = None
        self.right = None
        self.val = key 
        
# Driver code
"""         
        1
         \
          2
           \
            3
                 """
root = Node(1)
root.right = Node(2)
root.right.right = Node(3)

# This code is contributed by shivanisinghss2110
