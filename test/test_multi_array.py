'''
>>> from python_module import *
>>> x = multi_array2_float([8,8])
>>> x.dimensions
2
>>> x.shape
[8, 8]
>>> val = 0
>>> for i in range(8):
...   for j in range(8):
...       x[i,j] = val
...       val += 1
>>> print_matrix(x)
  0   1   2   3   4   5   6   7
  8   9  10  11  12  13  14  15
 16  17  18  19  20  21  22  23
 24  25  26  27  28  29  30  31
 32  33  34  35  36  37  38  39
 40  41  42  43  44  45  46  47
 48  49  50  51  52  53  54  55
 56  57  58  59  60  61  62  63
>>> y = x[0]
>>> y.dimensions
1
>>> y.shape
[8]
>>> for i in y:
...   print i
0.0
1.0
2.0
3.0
4.0
5.0
6.0
7.0
>>> y = x[2:-2, 2:-2]
>>> y.dimensions
2
>>> y.shape
[4, 4]
>>> print_matrix(y)
 18  19  20  21
 26  27  28  29
 34  35  36  37
 42  43  44  45
>>> y = x[0:2, 0:2]
>>> y.shape
[2, 2]
>>> print_matrix(y)
  0   1
  8   9
>>> y = x[0:2, -2:]
>>> y.shape
[2, 2]
>>> print_matrix(y)
  6   7
 14  15
>>> y = x[:, 3:-3]
>>> y.shape
[8, 2]
>>> print_matrix(y)
  3   4
 11  12
 19  20
 27  28
 35  36
 43  44
 51  52
 59  60


>>> x[0][0]
0.0
>>> x[1][0]
8.0
>>> x[0,0]
0.0
>>> x[1,0]
8.0

>>> x.reshape([4,16])
>>> print_matrix(x)
  0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
 16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31
 32  33  34  35  36  37  38  39  40  41  42  43  44  45  46  47
 48  49  50  51  52  53  54  55  56  57  58  59  60  61  62  63
>>> x.reshape([16,4])
>>> print_matrix(x)
  0   1   2   3
  4   5   6   7
  8   9  10  11
 12  13  14  15
 16  17  18  19
 20  21  22  23
 24  25  26  27
 28  29  30  31
 32  33  34  35
 36  37  38  39
 40  41  42  43
 44  45  46  47
 48  49  50  51
 52  53  54  55
 56  57  58  59
 60  61  62  63
>>> y = x[-1,:]
>>> for i in y:
...   print i,
60.0 61.0 62.0 63.0

>>> print_matrix(x[:3, 1:])
  1   2   3
  5   6   7
  9  10  11

>>> x[:3, :3] = [1,2,3,4,5,6,7,8,9]
>>> print_matrix(x)
  1   2   3   3
  4   5   6   7
  7   8   9  11
 12  13  14  15
 16  17  18  19
 20  21  22  23
 24  25  26  27
 28  29  30  31
 32  33  34  35
 36  37  38  39
 40  41  42  43
 44  45  46  47
 48  49  50  51
 52  53  54  55
 56  57  58  59
 60  61  62  63

>>> x[0,0] = 1
>>> x[0,0]
1.0
>>> x[-1,-1] = 2
>>> x[-1,-1]
2.0
>>> y = x[0]
>>> y[:] = [1,2,3,4]
>>> print_matrix(x)
  1   2   3   4
  4   5   6   7
  7   8   9  11
 12  13  14  15
 16  17  18  19
 20  21  22  23
 24  25  26  27
 28  29  30  31
 32  33  34  35
 36  37  38  39
 40  41  42  43
 44  45  46  47
 48  49  50  51
 52  53  54  55
 56  57  58  59
 60  61  62   2




>>> x = multi_array3_float([3,3,3])
>>> x[0,:,:] = [1,2,3,4,5,6,7,8,9]
>>> x[0,0,0]
1.0
>>> y = x[0,:,:]
>>> y.shape
[3, 3]
>>> y = x[:,0,:]
>>> y.shape
[3, 3]
>>> y[0,0]
1.0
>>> y[0,1]
2.0

>>> import pickle
>>> x == pickle.loads(pickle.dumps(x))
True
'''

def print_matrix(m):
  for i in m:
    for j in i:
      print '%3d' % j,
    print

def run(args = None):
    if args is not None:
        import sys
        sys.argv = args
    import doctest, test_multi_array
    return doctest.testmod(test_multi_array)

if __name__ == '__main__':
    import sys
    sys.exit(run()[0])
