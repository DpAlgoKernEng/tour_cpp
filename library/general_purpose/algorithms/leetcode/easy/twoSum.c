/**
 * @file              twoSum.c
 * @brief             两数之和
 * @version           0.1
 * @author            brightl birigtl3016@outlook.com
 * @date              2025.05.06
 * @copyright         Copyright (c) 2025.
 *
 * @note              Revision History:
 * <table>
 * <tr><th>Date       <th>Version         <th>Author          <th>Description
 * <tr><td>2025.05.06 <td>0.1             <td>brightl         <td>description
 * </table>
 */

/*
两数之和：
（1）给定一个整数数组 nums 和一个整数目标值 target，请你在该数组中找出 和 为目标值 target  的那 两个 整数，并返回它们的数组下标
（2）你可以假设每种输入只会对应一个答案，并且你不能使用两次相同的元素
（3）你可以按任意顺序返回答案

示例：
（1）示例1
    - 输入：nums = [2,7,11,15], target = 9
    - 输出：[0,1]
    - 解释：因为 nums[0] + nums[1] == 9 ，返回 [0, 1]

（2）示例2
    - 输入：nums = [3,2,4], target = 6
    - 输出：[1,2]

（3）示例3
    - 输入：nums = [3,3], target = 6
    - 输出：[0,1]

提示：
（1）2 <= nums.length <= 104
（2）-109 <= nums[i] <= 109
（3）-109 <= target <= 109
（4）只会存在一个有效答案

*/

#include <stdio.h>
#include <stdlib.h>

/*
方法一：暴力枚举
核心思想：
（1）通过双重循环遍历数组中所有可能的元素对 (nums[i], nums[j])，检查它们的和是否等于 target

思路及算法：
（1）最容易想到的方法是枚举数组中的每一个数 x，寻找数组中是否存在  target - x
（2）当我们使用遍历整个数组的方式寻找 target - x 时，需要注意到每一个位于 x 之前的元素都已经和 x 匹配过，因此不需要再进行匹配。而每一个元素不能被使用两次，所以我们只需要在 x 后面的元素中寻找 target - x

复杂度分析：
（1）时间复杂度：O(N^2)，其中 N 是数组中的元素数量。最坏情况下数组中任意两个数都要被匹配一次
 - O(1)	    数组随机访问	     操作次数与输入规模无关
 - O(logN)	二分查找	        每次操作将问题规模减半
 - O(N)	    遍历数组	        操作次数与输入规模成线性关系
 - O(NlogN)	快速排序、归并排序	分治法的典型复杂度
 - O(N^2)	  冒泡排序、暴力枚举两数之和	双重循环嵌套
 - O(2^N)	  穷举所有子集	     指数级增长，效率极低

（2）空间复杂度：O(1)
 - O(1)	    冒泡排序、暴力枚举两数之和	   只使用常数个变量
 - O(N)	    归并排序、哈希表优化的两数之和	需要额外数组或哈希表存储数据
 - O(N^2)	  动态规划（某些场景）	        二维数组存储中间结果
*/ 
/**
 * @brief             方法一：暴力枚举
 * @param   nums      给定整数数组 nums
 * @param   numSize   给定整数数组的长度 size
 * @param   target    目标值 target
 * @param   ret_size  返回值的数组的长度 ret size 
 * @return  int*      返回一个指向数组的指针 ret，数组和指针可以无缝切换
 *
 * @note              Revision History
 */
int *twoSum(int *nums, int numSize, int target, int* ret_size) {
  for (int i = 0; i < numSize; i++) {
    for (int j = i + 1; j < numSize; j++) {
      // nums[i] + nums[j] = target  
      if (nums[i] == target - nums[j]) {
        // 使用时再创建、与创建完成再使用的区别和场景
        int *ret = malloc(sizeof(int) * 2);
        ret[0] = i, ret[1] = j;
        *ret_size = 2;
        return ret;
      }
    }
  }
  *ret_size = 0;
  return NULL;
}

/*
方法二：哈希表

转换思路：
（1）查询 nums 数组中的元素 nums[i] 是否存在 nums[j] = target - nums[i]，每个 nums[i], nums[j] 组合只需要判断一次就可以了 
（2）双循环：判断 nums[i] 后面的数据 是否存在 nums[j] = target - nums[i] 前面的数据已经在循环中判断过了，无需重复判断，多了一层 for 循环
（3）hash表：判断 nums[i] 前面的数据 是否存在 nums[j] = target - nums[i] 前面的数据存储在 hash 表中复杂度为O(1)，减少一层 for 循环

思路及算法：
（1）注意到方法一的时间复杂度较高的原因是寻找 target - nums[i] 的时间复杂度过高。因此，我们需要一种更优秀的方法，能够快速寻找数组中是否存在目标元素。如果存在，我们需要找出它的索引
（2）使用哈希表，可以将寻找 target - nums[i] 的时间复杂度降低到从 O(N) 降低到 O(1)

复杂度分析
（1）时间复杂度：O(N)，其中 N 是数组中的元素数量。对于每一个元素 nums[i] 我们可以 O(1) 地寻找 target - nums[i]
（2）空间复杂度：O(N)，其中 N 是数组中的元素数量。主要为哈希表的开销

为什么需要二级指针？
（1）一级指针的情况：如果传递的是一个一级指针（如 struct hashTable* hashtable），那么函数内部只能操作该指针所指向的内容（即哈希表中的节点）。但在 freeHashTable 的逻辑中，我们不仅需要释放所有节点的内存，还需要将哈希表指针本身置为 NULL，以表示哈希表已经被清空
（2）二级指针的作用：使用二级指针可以让函数直接修改调用者传递的指针变量本身。这样，在函数内部释放完所有节点后，可以将调用者的指针置为 NULL，从而确保调用者不会误用已经被释放的哈希表
*/
#include "../../../tiny_lib/uthash/include/uthash.h"

/**
* @brief             定义 struct 结构体 hash 表
*
* @note              Revision History
*/
typedef struct nums {
  int key; // nums 元素 作为 key
  int idx; // nums 下标 作为 value
  UT_hash_handle hh; // uthash 的宏实现中硬编码了 hh 字段名，若用户结构体的句柄字段不叫 hh，宏将无法找到该字段，导致编译错误
} hashTable;

/**
* @brief             查询 hash 表
* @param   numshtableParam Description
* @param   key       Param Description
* @return  hashTable*Return Description
*
* @note              Revision History
*/
hashTable* findHashTable(hashTable** numshtable, int key) {
  hashTable* ret = NULL;
  HASH_FIND_INT(*numshtable, &key, ret);
  return ret;
}

/**
* @brief             插入数据到 hash 表
* @param   numshtableParam Description
* @param   idx       Param Description
* @param   key       Param Description
* @return  hashTable*Return Description
*
* @note              Revision History
*/
hashTable* insertHashTable(hashTable** numshtable, int idx, int key) {
  hashTable* ret = NULL;
  HASH_FIND_INT(*numshtable, &key, ret);
  if(ret == NULL){
    ret = (hashTable*)malloc(sizeof(hashTable));
    if (!ret) {
      perror("Memory allocation failed");
      exit(EXIT_FAILURE);
    }
    ret->key = key;
    ret->idx = idx;
    HASH_ADD_INT(*numshtable, key, ret);
  } else {
    ret->idx = idx;
  }
  return ret;
}

/**
* @brief             释放 hash表
* @param   numshtableParam Description
* @return  int       Return Description
*
* @note              Revision History
*/
void freeHashTable(hashTable** numshtable) {
  hashTable *current = NULL, *tmp = NULL; 
  HASH_ITER(hh, *numshtable, current, tmp) {
    HASH_DEL(*numshtable, current);  
    free(current);
  }
  return;
}

/**
* @brief             方法二：哈希表
* @param   nums      给定整数数组 nums
* @param   numSize   给定整数数组的长度 size
* @param   target    目标值 target
* @param   ret_size  返回值的数组的长度 ret size 
* @return  int*      返回一个指向数组的指针 ret，数组和指针可以无缝切换
*
* @note              Revision History
*/
int *twoSumHash(int *nums, int numSize, int target, int* ret_size) {
  if (nums == NULL || numSize < 2 || ret_size == NULL) {
    *ret_size = 0;
    return NULL;
  }
  hashTable* numshtable = NULL;
  // 不用先将元素都添加到 hash表里去，直接遍历一次查询，如果 target - nums[i] 不在 hash表里，就说明之前的数据都不满足，将这个数据存放在 hash表里 供下次查询，如果存在就说明找到了
  for (int i = 0; i < numSize; i++) {
    hashTable* retht = findHashTable(&numshtable, target - nums[i]);
    if (retht != NULL) {
      int* ret = (int*)malloc(sizeof(int)*2);
      ret[0] = i;
      ret[1] = retht->idx;
      *ret_size = 2;
      freeHashTable(&numshtable);
      return ret;
    }
    insertHashTable(&numshtable,i,nums[i]);
  }
  *ret_size = 0;
  freeHashTable(&numshtable);
  numshtable = NULL; // free 释放后立即置空指针，避免悬空指针
  return NULL;
}

/**
* @brief             测试两数之和
* @return  int       Return Description
*
* @note              Revision History
*/
int main(void) {
    int nums1[] = {2, 7, 11, 15};
    int target1 = 9;
    int size = 0;
    int* ret_size = &size;
    int* ret1 = twoSumHash(nums1, 4, target1, ret_size);
    printf("ret1 : ret1[0] = %d, ret1[1] = %d\n", ret1[0], ret1[1]);
    printf("ret_size = %d\n", *ret_size);
    free(ret1);

    int nums2[] = {3, 2, 4};
    int target2 = 6;
    int* ret2 = twoSumHash(nums2, 3, target2, ret_size);
    printf("ret2 : ret2[0] = %d, ret2[1] = %d\n", ret2[0], ret2[1]);
    printf("ret_size = %d\n", *ret_size);
    free(ret2);   
    
    int nums3[] = {3, 3};
    int target3 = 6;
    int* ret3 = twoSumHash(nums3, 2, target3, ret_size);
    printf("ret3 : ret3[0] = %d, ret3[1] = %d\n", ret3[0], ret3[1]);
    printf("ret_size = %d\n", *ret_size);
    free(ret3); 
}