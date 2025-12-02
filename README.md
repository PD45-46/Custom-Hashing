# Custom-Hashing

## Background Information

My implementation of hashing methods are based off of content taught in Australian National University's course COMP3600 (Algorithms) and Carnegie Mellon University's [Universal and Perfect Hashing Lecture](https://www.cs.cmu.edu/~avrim/451f11/lectures/lect1004.pdf). 

### What is Hashing? 

Just an initial note for the reader. I will not be going super in depth when explaining hashing as I just want to give the reader a basic understanding of how everything works. I want the reader to be able to understand but not be overwhelmed with the information so that they can also participate in my findings. 

#### Overview

Say that we want to store unique pieces of information (of an arbitrary size) into some kind of structure. If we were to use the classical array, we’d run into run time issues when trying to recover a specific key. In our worst case scenario we would have to traverse the entire array in order to recover the desired key; $O(n)$ time complexity, where the array contains $n$ elements. But say that somehow we were able to establish a key identifier function that is able to create a (somewhat) unique ‘code’ and place our key in the slot of a data structure with respect to that code. This way the time complexity is only dependent on the key identifier function as we can reuse that function to locate our desired key. Hashing is the basic idea of using a hash function to create unique codes that we can reference when trying to find keys in our data structure, note that this gives us a lookup amortized time complexity of $O(1)$ since the hash function itself should be able to perform in constant time. Note that amortized simply means the most average case. 

The generic  hash function would take the shape of: 
    
$h(x) = (ax + b) \bmod p$

Where $a$ and $b$ are constants, $x$ is our input value and $p$ is some (uncommon) prime. 

My project will only be covering static hashing methods, meaning that the key set will remain unchanged and so will not have to worry about insert and delete operations when dealing with the hash table.

#### How do Hash Functions work? 

#### Collision Resolution Schemes 

#### What is Universal Hashing? 

#### What is Perfect Hashing?  

### Why use C? 

### How to run the code



## Implementing Theory 



## Testing The Implementation 




## Running Benchmarks 


### Bench analysis



