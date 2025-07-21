# Code from https://builtin.com/data-science/full-tree?utm_source=chatgpt.com

#creating a class to represent a binary tree node
class Node:
    def __init__(self, data):
        self.data = data
        self.left = None
        self.right = None
        
#creating the complete binary tree
root = Node(1)
root.left = Node(2)
root.right = Node(3)
root.left.left = Node(4)
root.left.right = Node(5)
root.right.left = Node(6)

#define function to count number of nodes
def node_count(root):
    if root is None:
        return 0
    return (1 + node_count(root.left) + node_count(root.right))

#define function to check if complete binary tree
def is_completetree(root, index, node_amount):
    if root is None:
        return True
    if index >= node_amount:
        return False
    return (is_completetree(root.left, 2 * index + 1, node_amount) and is_completetree(root.right, 2 * index + 2, node_amount))

#define node_total and initialize index variable
node_total = node_count(root)
index = 0

#printing result of the checking function
if is_completetree(root, index, node_total):
    print("This is a complete binary tree.")
else:
    print("This is not a complete binary tree.")

#define function for inorder traversal of the tree
def inorder_traversal(root):
    if root:
        inorder_traversal(root.left)
        print(root.data, end=" ")
        inorder_traversal(root.right)

#displaying the tree traversal
print("Inorder Traversal: ", end="")
inorder_traversal(root)

#Output for example complete binary tree: 'Inorder Traversal: 4 2 5 1 6 3'
