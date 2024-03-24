import random
import math

# Function to generate a single test case
def generate_test_case():
    # Define the range for coordinates and rotation
    centroid_x = random.randint(-30000, 30000)
    centroid_y = random.randint(-30000, 30000)
    min_coord, max_coord = -10000, +10000
    min_rotation, max_rotation = int(-math.pi * 1000), int(math.pi * 1000)

    # Generate random set sizes between 10 and 30
    size1 = random.randint(10, 30)
    size2 = size1 + random.randint(10, 15)
    size1 = size2
    rot1 = random.randint(min_rotation, max_rotation)
    rot2 = random.randint(min_rotation, max_rotation)

    print("Rot1: ", rot1, " Rot2: ", rot2, "\nSize1: ", size1, "Size2: ", size2, "\nMinCoor: ", min_coord, "MaxCoor: ", max_coord, "MinRot: ", min_rotation, "MaxRot: ", max_rotation)


    # Generate points for set1 with random coordinates and rotation
    set2 = [[centroid_x + random.randint(min_coord, max_coord),
             centroid_y - random.randint(min_coord, max_coord),
             rot2] for _ in range(size2)]

    # Randomly shuffle set1 to create set2 (different order)
    #set1 = random.sample(set2, size1)
    # set1 = [[0,0,0]]*size1
    # for i in range(size1):
    #     set1[i][0] = set2[i][0]
    #     set1[i][1] = set2[i][1]
    #     set1[i][2] = set2[i][2]
    # random.shuffle(set1)

    set1=[]
    # Rotate set2 by rot2
    # translate set2 by random amount
    trans_x = random.randint(min_coord//4, max_coord//4)
    trans_y = random.randint(min_coord//4, max_coord//4)
    print("trans_x: ", trans_x, "trans_y: ", trans_y)
    for i in range(size1):
        x, y = set2[i][0], set2[i][1]
        x = x + trans_x
        y = y + trans_y
        a = int( x * math.cos(rot1/1000) - y * math.sin(rot1/1000) ) #+ trans_x )
        b = int( x * math.sin(rot1/1000) + y * math.cos(rot1/1000) ) #+ trans_y )
        c = int( rot2 + rot1 )
        print("xy:", x, y, " ->", a,b)
        set1.append([a, b, c])
        
    return set1, set2


def print_set_cpp(set):
    print("std::vector<Point> set = {", end="")
    for point in set:
        print("{", point[0], ", ", point[1], ", ", point[2], "}, ", end="")
    print("};")

# Example usage to generate a test case
set1, set2 = generate_test_case()
print("Set 1:")
print(set1)
print("Set 2:")
print(set2)

print("Set 1 CPP:")
print_set_cpp(set1)
print("Set 2 CPP:")
print_set_cpp(set2)