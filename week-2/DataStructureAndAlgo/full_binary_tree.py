# Code from https://builtin.com/data-science/full-tree?utm_source=chatgpt.com

#creating a class to represent a binary tree node
class Node:
    def __init__(self, data):
        self.data = data
        self.left = None
        self.right = None
        
#creating the full binary tree
root = Node(1)
root.left = Node(2)
root.right = Node(3)
root.left.left = Node(4)
root.left.right = Node(5)
root.right.left = Node(6)
root.right.right = Node(7)

#define function to check if a full binary tree
def is_fulltree(root):
    if root is None:
        return True
    if (root.left is None and root.right is not None) or (root.left is not None and root.right is None):
        return False
    return is_fulltree(root.left) and is_fulltree(root.right)

#printing result of the checking function
if is_fulltree(root):
    print("This is a full binary tree.")
else:
    print("This is not a full binary tree.")

#define function for inorder traversal of the tree
def inorder_traversal(root):
    if root:
        inorder_traversal(root.left)
        print(root.data, end=" ")
        inorder_traversal(root.right)

#displaying the tree traversal
print("Inorder Traversal: ", end="")
inorder_traversal(root)

#Output for example full binary tree: 'Inorder Traversal: 4 2 5 1 6 3 7'
