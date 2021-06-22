需要的東西：

1. 字串比對演算法一套（無腦Rabin-Karp？）
2. disjoint set：
    1. 如果出現過去沒出現過的，兩個人去旁邊造一個set。
    2. 如果出現已經出現過的，就併過去
        1. 既然最後已經確定要呈現出最大的 size，那用 union by rank，再用一個count
        2. Path compression

Pseudo code：
```pseudocode
input: mids[len] //from 0 to len-1
output: arr[2], arr[0] = numbers of sets, arr[1] = size of the largest set

function find

function union_by_rank

function compareMax(a, b)
	if a > b
		return a
	else
		return b

function main()
	for(int i = 0; i< len; i++)
		check mail[mids[i].from] and mail[mids[i].to] exists or not
			if both exists and belongs to same set
				do nothing
			else if both exists but belongs to different sets
				union the two sets by rank
				size = A_size + B_size
				compare the largest size and the size
				setCount = setCount - 1
			else if one exist but the other not
				union the unexixsted one to the set that the existed one belongs to
				size = A_size + 1
				compare the largest size and the size
			else //both are not existed
				makeset(mail[mids[i]].from, mail[mids[i]].to)
				setCount = setCount + 1
				size = 2
				compare the largest size and the size



add:
1. the number of sets(setCount)
2. the size of the largest set(largestCount)
to arr[0], arr[1]
return arr

```


