# Binary Parser 
Parsing things like .npy files in C

## Numpy
Numpy has two basic binary file store methods, .npy files which are the raw
binary information of the numpy arry with a header on top and .npz files
which are a container for multiple .npy files.

If we call
```bash
python3 scripts/generate_npy.py -N 32 -n 1 -s 654
hexdump -C data/binary0.npy
```
We get the following hexdump
```
00000000  93 4e 55 4d 50 59 01 00  76 00 7b 27 64 65 73 63  |.NUMPY..v.{'desc|
00000010  72 27 3a 20 27 3c 66 34  27 2c 20 27 66 6f 72 74  |r': '<f4', 'fort|
00000020  72 61 6e 5f 6f 72 64 65  72 27 3a 20 46 61 6c 73  |ran_order': Fals|
00000030  65 2c 20 27 73 68 61 70  65 27 3a 20 28 31 36 2c  |e, 'shape': (16,|
00000040  29 2c 20 7d 20 20 20 20  20 20 20 20 20 20 20 20  |), }            |
00000050  20 20 20 20 20 20 20 20  20 20 20 20 20 20 20 20  |                |
*
00000070  20 20 20 20 20 20 20 20  20 20 20 20 20 20 20 0a  |               .|
00000080  85 b6 0e bf a3 01 9b be  f3 2a 96 40 6d 64 65 3f  |.........*.@mde?|
00000090  91 b5 ba 40 4f 7c 84 3f  af a6 99 3f 3e 60 1a c0  |...@O|.?...?>`..|
000000a0  0d 43 3d 40 10 21 44 40  fc fe a9 40 20 21 70 3f  |.C=@.!D@...@ !p?|
000000b0  95 da e1 3d 56 c0 29 3f  af 37 47 40 4f 7c 20 40  |...=V.)?.7G@O| @|
000000c0
```

For our simply numpy generation the header looks as follows
```
.NUMPY..v.{'descr': '<f4', 'fortran_order': False, 'shape': (16,), }  
```

The `'<f4'` refers to the fact that our array is filled with 32 bit floating point
numbers if we had kept them as the default 64 bit floats then it would be
`'<f8'`.

The `'fortran_order'` refers to whether the entries are stored in row major
(which is how the C programming langauge stores them) or in column major
(which is what fortran uses) form.

Unsurprisingly iterating through a `.npy` file with numpy as very easy, we can
just load the entire file as a numpy array
```python
arr = np.load("data/bianry0.npy")

print(type(arr))
print(arr.shape)
for val in np.nditer(arr):
    print(float(val))
```
yields the following output
```
<class 'numpy.ndarray'>
(16,)
-0.5574725270271301
-0.30274686217308044
4.692742824554443
0.8960636258125305
5.8346638679504395
1.0350435972213745
1.2003992795944214
-2.4121241569519043
2.9572174549102783
3.0645179748535156
5.312376022338867
0.9380054473876953
0.11028019338846207
0.6630910634994507
3.1127736568450928
2.507587194442749
```

We'd like to iterate through the binary file in a word by word (i.e. 8 bytes on our 64 bit operating
system) fashion, instead of the way that hexdump shows them (2 words per row), for that it's more
convenient to use `xxd` and pass how many bits we want to see.
```
xxd -c 8 -g 1 data/binary0.npy
```
```
00000000: 93 4e 55 4d 50 59 01 00  .NUMPY..
00000008: 76 00 7b 27 64 65 73 63  v.{'desc
00000010: 72 27 3a 20 27 3c 66 34  r': '<f4
00000018: 27 2c 20 27 66 6f 72 74  ', 'fort
00000020: 72 61 6e 5f 6f 72 64 65  ran_orde
00000028: 72 27 3a 20 46 61 6c 73  r': Fals
00000030: 65 2c 20 27 73 68 61 70  e, 'shap
00000038: 65 27 3a 20 28 31 36 2c  e': (16,
00000040: 29 2c 20 7d 20 20 20 20  ), }    
00000048: 20 20 20 20 20 20 20 20          
00000050: 20 20 20 20 20 20 20 20          
00000058: 20 20 20 20 20 20 20 20          
00000060: 20 20 20 20 20 20 20 20          
00000068: 20 20 20 20 20 20 20 20          
00000070: 20 20 20 20 20 20 20 20          
00000078: 20 20 20 20 20 20 20 0a         .
00000080: 85 b6 0e bf a3 01 9b be  ........
00000088: f3 2a 96 40 6d 64 65 3f  .*.@mde?
00000090: 91 b5 ba 40 4f 7c 84 3f  ...@O|.?
00000098: af a6 99 3f 3e 60 1a c0  ...?>`..
000000a0: 0d 43 3d 40 10 21 44 40  .C=@.!D@
000000a8: fc fe a9 40 20 21 70 3f  ...@ !p?
000000b0: 95 da e1 3d 56 c0 29 3f  ...=V.)?
000000b8: af 37 47 40 4f 7c 20 40  .7G@O| @
```
As a sidenote, note that there are a lot of `0x20` entries after our header,
those are there to align our binary file to the cache line size, which is 8 byte.

We can see this easier if we print the binary as cahce line rows
```
xxd -c 64 -g 1 data/binary0.npy
```
but those barely fit on the screen:
```
00000000: 93 4e 55 4d 50 59 01 00 76 00 7b 27 64 65 73 63 72 27 3a 20 27 3c 66 34 27 2c 20 27 66 6f 72 74 72 61 6e 5f 6f 72 64 65 72 27 3a 20 46 61 6c 73 65 2c 20 27 73 68 61 70 65 27 3a 20 28 31 36 2c  .NUMPY..v.{'descr': '<f4', 'fortran_order': False, 'shape': (16,

00000040: 29 2c 20 7d 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 0a  ), }                                                           .

00000080: 85 b6 0e bf a3 01 9b be f3 2a 96 40 6d 64 65 3f 91 b5 ba 40 4f 7c 84 3f af a6 99 3f 3e 60 1a c0 0d 43 3d 40 10 21 44 40 fc fe a9 40 20 21 70 3f 95 da e1 3d 56 c0 29 3f af 37 47 40 4f 7c 20 40  .........*.@mde?...@O|.?...?>`...C=@.!D@...@ !p?...=V.)?.7G@O| @
```

### The .npy standard
The previous text was based on personal exploration of the .npy binary, the following is me working through the standard itself. A .npy file has the following header
```
00000000: 93 4e 55 4d 50 59 01 00  .NUMPY..
00000008: 76 00 7b 27 64 65 73 63  v.{'desc
00000010: 72 27 3a 20 27 3c 66 34  r': '<f4
00000018: 27 2c 20 27 66 6f 72 74  ', 'fort
00000020: 72 61 6e 5f 6f 72 64 65  ran_orde
00000028: 72 27 3a 20 46 61 6c 73  r': Fals
00000030: 65 2c 20 27 73 68 61 70  e, 'shap
00000038: 65 27 3a 20 28 31 36 2c  e': (16,
00000040: 29 2c 20 7d 20 20 20 20  ), }    
00000048: 20 20 20 20 20 20 20 20          
*
00000078: 20 20 20 20 20 20 20 0a         .
```
The first 6 bytes `93 4e 55 4d 50 59` are `\x93NUMPY`, then one unsigned byte for major `01` and one for minor `00` version. From this we can see that .npy file I'm working with is done with the version 1.0 standard. That makes up the first word.

The first two bytes of the second word are a little endian encoding of the length of the header, which has the constant name `HEADER_LEN`, in our case we have `HEADER_LEN` defined by `76 00`.

In our case we have
```
{'descr': '<f4', 'fortran_order': False, 'shape': (16,), }
```
as the corresponding dictionary.

# References
* Numpy documentation
    * Save Function
        * https://numpy.org/doc/stable/reference/generated/numpy.save.html
    * .npy Format
        * https://numpy.org/devdocs/reference/generated/numpy.lib.format.html#
    * NEP1 - A simple file format for NumPy arrays (from 2007-12-20)
        * https://numpy.org/neps/nep-0001-npy-format.html
* JSON Standard
    * https://www.json.org/json-en.html