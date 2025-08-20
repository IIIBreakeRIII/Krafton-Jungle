# Code from https://www.geeksforgeeks.org/dsa/skewed-binary-tree/

# Python3 implementation of the above approach 

# Class that represents an individual
# node in a Binary Tree 
class Node: 
    def __init__(self, key): 
        
        self.left = None
        self.right = None
        self.val = key 
        
# Driver code

"""         1
           /
          2
         /
        3     """
root = Node(1)
root.left = Node(2)
root.left.left = Node(2)

# This code is contributed by dhruvsantoshwar
