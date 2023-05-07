# Congestion-based Global Router
>Ching-Cheng Wang
>
## How to Compile & Run
### Compile

```unix=
$ make
```

### Run
```unix=
$ ./hw5 [txt_file] [result_file]
```
For example : 
```unix=
$ ./hw5 ../testcase/ibm01.modified.txt ../output/ibm01.result
```

## Flow
```flow
st=>start: input file
e=>end: output file
op=>operation: maze routing
op2=>operation: rip-up overflow net
op3=>operation: reroute overflow net
op4=>operation: shuffle routing result
cond=>condition: overflow?
cond2=>condition: fail times>150
cond3=>condition: time>limit

st->op->op2->op3->cond2
op4->op2
cond(yes)->cond3
cond(no)->e
cond2(yes)->op4
cond2(no)->cond
cond3(yes)->e
cond3(no)->op2

```

## Implementation
### Flow
1. Read file.
2. I implemented A* search and maze routing and found that using maze routing for the first round of routing is faster. During maze routing, I simply used the labeling method based on the Lee algorithm in the lecture notes.
3. Remove the overflow net and perform a rip-up and reroute. Starting from the source, use BFS to expand in each direction and calculate the cost for each direction. Push the four grid points in each direction into a priority queue. In each iteration, pop a point from the queue and perform BFS until the sink is reached.
4. Starting from the sink, use the parent stored during BFS to backtrace to the source.
5. If multiple reroutes are performed without reducing the total overflow, the grid may be slightly disrupted or even increase some overflow so that my rip-up and reroute algorithm can find some paths that are different from the original ones.
6. If there is still overflow and within the runtime limit, go to step 3.
7. Output file.
### Cost function
I made some modifications to the cost function of nthu-route 2.0

| Notation | Explantion                              |
|:-------- |:--------------------------------------- |
| $e_h$    | edge history cost                       |
| $e_d$    | edge demand                             |
| $e_c$    | edge capacity                           |
| $cost_p$ | parent cost                             |
| $m$      | current point->sink manhanttan distance |
| $wl$     | wirelength                              |
| $\alpha$ | constant                                |
| $c_p$    | point congestion cost                   |
| $cost$   | move cost                               |

$c_p=cost_p+\alpha\times e_h\times(\frac{e_d+1}{e_c})^5$
$cost=wl+m+c_p$

## Runtime limit
For testcases that take a long time, I set a time limit function
$t :$ Runtime of one reroute
$limit=t\times100$

## Main Result
| benchmark        | ibm01 | ibm02  | ibm04  |
|:---------------- |:-----:|:------:|:------:|
| # of overflow    |  18   |   0    |  161   |
| max overflow     |   3   |   0    |   7    |
| total wirelength | 60503 | 161476 | 159406 |
| runtime(s)       | 32.96 | 41.14  | 180.33 |


## Experimental Result
### Test against $\alpha$ (Quality experiment)
1. $\alpha=1$

| benchmark        | ibm01 | ibm02  | ibm04  |
|:---------------- |:-----:|:------:|:------:|
| # of overflow    |  26   |   2    |  161   |
| max overflow     |   5   |   1    |   7    |
| total wirelength | 59763 | 159254 | 159406 |
| runtime(s)       | 34.81 |  43.62 | 180.33 |

2. $\alpha=2$

| benchmark        | ibm01 | ibm02  | ibm04  |
|:---------------- |:-----:|:------:|:------:|
| # of overflow    |  16   |   0    |  170   |
| max overflow     |   3   |   0    |   6    |
| total wirelength | 60531 | 161476 | 160686 |
| runtime(s)       | 32.96 | 41.14  | 181.88 |

3. $\alpha=2.5$

| benchmark        | ibm01 | ibm02  | ibm04  |
|:---------------- |:-----:|:------:|:------:|
| # of overflow    |  21   |   0    |  178   |
| max overflow     |   3   |   0    |   4    |
| total wirelength | 60503 | 162384 | 160734 |
| runtime(s)       | 37.52 | 47.03  | 181.88 |

![](https://i.imgur.com/IzdutTI.png)

![](https://i.imgur.com/zbodgHU.png)

### Runtime Experiments
#### Method A
Regarding runtime, since each rip-up reroute requires referencing edge history, and my edge class contains a lot of elements, it would take a considerable amount of time to update the cost if I were to store it inside the edge class. In particular, reverting the cost back if it does not improve would take a lot of time. Therefore, I have created two additional two-dimensional float arrays to store the history cost values.

#### Method B
In A* search, since BFS is required to find neighbors each time, I created a lookup table to quickly find neighbors and speed up the search.

The table below illustrates a significant difference in runtime before and after the improvement!

| benchmark        | ibm01 | ibm02  | ibm04  |
|:---------------- |:-----:|:------:|:------:|
| # of overflow    |  18   |   0    |  161   |
| max overflow     |   3   |   0    |   7    |
| total wirelength | 60503 | 161476 | 159406 |
| runtime(s) w/o A | 80.47 | 100.14 | 392.33 |
| runtime(s) w/o B | 62.14 | 71.14  | 256.07 |
| runtime(s)       | 32.96 | 41.14  | 180.33 |
![](https://i.imgur.com/CBMVeJ6.png)

## Bonus
I use OpenGL to draw a congestion map, and since my overflows are not significant, I only divided it into three color levels.
### Color Table
![](https://i.imgur.com/JpIMtLY.png)

### ibm01
![](https://i.imgur.com/E08GwWu.png)

### ibm02
![](https://i.imgur.com/xnw2crM.png)

### ibm04
![](https://i.imgur.com/3YF4dtJ.png)



## Reference
1. NTHU-Route 2.0: A Robust Global Router for Modern Designs
2. PathFinder: A Negotiation-Based Performance-Driven Router for FPGAs
3. FastRoute 2.0: A High-quality and Efficient Global Router
4. BoxRouter: A New Global Router Based on Box Expansion and Progressive ILP
