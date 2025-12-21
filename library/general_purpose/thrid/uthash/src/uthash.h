/*
Copyright (c) 2003-2025, Troy D. Hanson  https://troydhanson.github.io/uthash/
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
头文件保护（Header Guard）的预处理指令，主要作用是防止头文件被重复包含，避免编译错误，等价 #pragma once

// uthash.h 文件
#ifndef UTHASH_H // 如果 UTHASH_H 未定义
#define UTHASH_H // 定义 UTHASH_H
// 这里是头文件的实际内容（结构体、函数声明等）
#endif // 结束条件编译

扩展：#pragma once 现代编译器支持更简洁的指令 #pragma once，作用相同，但 #ifndef 是标准C支持的，而 #pragma once 是编译器扩展，兼容性稍差。因此开源项目更常用 #ifndef 方式
*/
#ifndef UTHASH_H
#define UTHASH_H

/*
定义 uthash 版本
*/
#define UTHASH_VERSION 2.3.0

/*
头文件包含
*/
#include <string.h>   /* memcmp, memset, strlen */
#include <stddef.h>   /* ptrdiff_t */
#include <stdlib.h>   /* exit */

/*
头文件包含的跨平台兼容性：
在支持 C99 标准的环境下（多数现代系统），默认使用 <stdint.h>
在不支持 <stdint.h> 的旧系统（如某些嵌入式设备）中，用户可定义 HASH_NO_STDINT 并自行实现类型

这段代码是用于条件性包含 <stdint.h> 头文件的预处理指令，目的是根据用户配置灵活处理固定宽度整数类型（如 uint8_t、uint32_t）的定义，增强代码的可移植性
*/
#if defined(HASH_NO_STDINT) && HASH_NO_STDINT
/* The user doesn't have <stdint.h>, and must figure out their own way
   to provide definitions for uint8_t and uint32_t. */
#else
#include <stdint.h>   /* uint8_t, uint32_t */
#endif

/*
类型推断的兼容性：
这段代码主要用于跨编译器兼容性处理，通过条件编译实现两种功能：
自动选择类型推断方式（decltype 或 __typeof），适配不同编译器
定义安全的赋值宏 DECLTYPE_ASSIGN，确保类型匹配或绕过类型检查（针对不支持类型推断的旧编译器）

条件定义 DECLTYPE
MSVC 编译器：
  VS2010+ 且 C++模式：支持 decltype，直接使用
  旧版本或 C 模式：定义 NO_DECLTYPE，禁用类型推断
Elbrus 编译器：使用 __typeof（GNU 扩展）
其他明确列出的编译器（Borland、IAR ARM 等）：禁用类型推断
默认情况（GCC、Clang、SunPro 等）：使用 __typeof

处理 NO_DECLTYPE 情况：
NO_DECLTYPE 不支持类型推断分支：
  DECLTYPE(x)：定义为空，编译时直接移除类型信息
  DECLTYPE_ASSIGN：
  通过 双指针强制转换 实现赋值，绕过类型检查
  原理：将目标变量地址转为 char**，再解引用并赋值为 src 的地址（强制为 char*）
  风险：可能引发未定义行为（类型不匹配时崩溃或数据损坏）

支持类型推断的分支：
DECLTYPE_ASSIGN：
  使用 DECLTYPE(dst) 获取 dst 的类型，并将 src 强制转换为此类型后赋值，确保类型安全

哈希表实现：
在泛型数据结构中，需将不同类型的数据插入哈希表。通过 DECLTYPE_ASSIGN 确保赋值操作类型安全或绕过类型检查（旧编译器）

扩展：do { ... } while (0) 的作用
将多行代码包裹为单语句：确保宏展开后无论是否在 if/else 等代码块中，都能正确解析
避免宏展开后的副作用：提供独立的作用域，防止变量名冲突
*/
/* These macros use decltype or the earlier __typeof GNU extension.
   As decltype is only available in newer compilers (VS2010 or gcc 4.3+
   when compiling c++ source) this code uses whatever method is needed
   or, for VS2008 where neither is available, uses casting workarounds. */
#if !defined(DECLTYPE) && !defined(NO_DECLTYPE)
#if defined(_MSC_VER)   /* MS compiler */
#if _MSC_VER >= 1600 && defined(__cplusplus)  /* VS2010 or newer in C++ mode */
#define DECLTYPE(x) (decltype(x))
#else                   /* VS2008 or older (or VS2010 in C mode) */
#define NO_DECLTYPE
#endif
#elif defined(__MCST__)  /* Elbrus C Compiler */
#define DECLTYPE(x) (__typeof(x))
#elif defined(__BORLANDC__) || defined(__ICCARM__) || defined(__LCC__) || defined(__WATCOMC__)
#define NO_DECLTYPE
#else                   /* GNU, Sun and other compilers */
#define DECLTYPE(x) (__typeof(x))
#endif
#endif

#ifdef NO_DECLTYPE
#define DECLTYPE(x)
#define DECLTYPE_ASSIGN(dst,src)                                                 \
do {                                                                             \
  char **_da_dst = (char**)(&(dst));                                             \
  *_da_dst = (char*)(src);                                                       \
} while (0)
#else
#define DECLTYPE_ASSIGN(dst,src)                                                 \
do {                                                                             \
  (dst) = DECLTYPE(dst)(src);                                                    \
} while (0)
#endif

/*
自定义内存管理和字符串操作函数的宏定义：
这段代码是 UTHash 库中用于自定义内存管理和字符串操作函数的宏定义，目的是允许用户替换默认的库行为，增强代码的灵活性和可移植性

设计目的：
解耦依赖：允许用户替换标准库函数，避免库与特定内存管理实现强绑定
调试与追踪：用户可插入日志、统计内存使用量或检测内存泄漏
嵌入式系统适配：在资源受限环境中，可使用静态内存池或硬件加速操作
性能优化：替换为更高效的内存管理实现（如 tcmalloc、jemalloc）
*/
#ifndef uthash_malloc
#define uthash_malloc(sz) malloc(sz)      /* malloc fcn                      */
#endif
#ifndef uthash_free
#define uthash_free(ptr,sz) free(ptr)     /* free fcn                        */
#endif
#ifndef uthash_bzero
#define uthash_bzero(a,n) memset(a,'\0',n)
#endif
#ifndef uthash_strlen
#define uthash_strlen(s) strlen(s)
#endif

/*
可自定义哈希表函数宏定义：
这段代码是 UTHash 库中用于自定义哈希表行为的宏定义，主要涉及哈希函数、键比较函数和哈希表扩容回调的配置

哈希函数 HASH_FUNCTION 定义哈希值的生成方式，默认行为：使用 Jenkins 哈希算法 (HASH_JEN)，这是 UTHash 的默认哈希函数，自定义场景：替换为其他哈希算法（如 MurmurHash、CityHash 等），需保证函数签名匹配

键比较函数 HASH_KEYCMP 定义键的比较逻辑（用于处理哈希冲突），使用 memcmp 逐字节比较内存内容，自定义场景：当键为复杂结构或需要特殊比较逻辑时（如字符串忽略大小写、结构体部分字段比较等）

哈希表扩容回调 
uthash_noexpand_fyi 作用：当哈希表未触发扩容时的回调通知、自定义场景：调试或监控哈希表负载状态
uthash_expand_fyi 作用：当哈希表触发扩容时的回调通知、自定义场景：统计扩容次数或记录性能指标

设计目的与使用规则
灵活性：用户可通过自定义宏替换默认行为，适应不同场景需求（如性能优化、调试、特殊键类型）
性能调优：选择更高效的哈希函数（如对抗哈希碰撞攻击）、优化键比较逻辑（如减少 memcmp 的冗余计算）
调试与监控：通过回调监控哈希表扩容行为、记录内存使用或负载状态
使用规则：自定义宏需在包含 uthash.h 前定义、确保自定义函数与默认接口参数兼容
*/
#ifndef HASH_FUNCTION
#define HASH_FUNCTION(keyptr,keylen,hashv) HASH_JEN(keyptr, keylen, hashv)
#endif

#ifndef HASH_KEYCMP
#define HASH_KEYCMP(a,b,n) memcmp(a,b,n)
#endif

#ifndef uthash_noexpand_fyi
#define uthash_noexpand_fyi(tbl)          /* can be defined to log noexpand  */
#endif
#ifndef uthash_expand_fyi
#define uthash_expand_fyi(tbl)            /* can be defined to log expands   */
#endif

/*
自定义内存分配失败处理逻辑：
这段代码是 UTHash 库中用于控制内存分配失败（OOM，Out Of Memory）处理策略的条件编译逻辑，允许用户选择优雅恢复或直接终止程序

默认行为：内存分配失败时，强制终止程序（exit(-1)），避免使用无效哈希表
可选配置：通过定义 HASH_NONFATAL_OOM 为非零值，允许程序尝试从 OOM 中恢复，继续运行、HASH_NONFATAL_OOM：默认值为 0，即内存分配失败时直接终止程序。若需启用恢复能力，需手动定义为 1

#if HASH_NONFATAL_OOM
// 非致命 OOM 分支
// uthash_nonfatal_oom(obj)：OOM 发生时的回调函数，默认无操作。用户可扩展为记录日志、释放备用资源等
// HASH_RECORD_OOM(oomed)：标记 OOM 状态，将变量 oomed 设为 1，供后续代码检查
// IF_HASH_NONFATAL_OOM(x)：条件编译开关，仅在非致命模式下保留代码 x。用于包裹恢复逻辑

#else
// 致命 OOM 分支（默认）
// uthash_fatal(msg)：OOM 发生时的终止函数，默认调用 exit(-1)。用户可重定义为抛出异常或记录错误
// HASH_RECORD_OOM(oomed)：直接触发终止操作，忽略 oomed 参数
// IF_HASH_NONFATAL_OOM(x)：定义为空，编译时移除 x 代码，确保恢复逻辑不会在致命模式下存在

#endif

使用场景对比：
非致命模式 (HASH_NONFATAL_OOM=1)：
  内存分配失败行为：标记 oomed 变量，程序可继续运行
  用户责任：需检查 oomed 并处理错误（如回滚操作）
  自定义扩展：通过 uthash_nonfatal_oom 添加日志或恢复逻辑
  适用场景：高可用服务、嵌入式系统、需优雅降级的场景
致命模式（默认）：
  内存分配失败行为：立即终止程序
  用户责任：无需额外处理
  自定义扩展：通过 uthash_fatal 自定义终止行为
  适用场景：简单应用、调试环境、内存错误不可接受的场景

设计目的：
  灵活性：允许用户根据场景选择内存错误处理策略
  安全性：默认终止程序避免未定义行为，适合大多数简单应用
  可扩展性：通过回调函数集成到现有错误处理框架（如日志、异常）
注意事项：
  状态一致性：非致命模式下，哈希表可能处于无效状态，需避免继续操作
  性能影响：非致命模式需额外分支检查 oomed，轻微增加运行时开销
*/
#ifndef HASH_NONFATAL_OOM
#define HASH_NONFATAL_OOM 0
#endif

#if HASH_NONFATAL_OOM
/* malloc failures can be recovered from */

#ifndef uthash_nonfatal_oom
#define uthash_nonfatal_oom(obj) do {} while (0)    /* non-fatal OOM error */
#endif

#define HASH_RECORD_OOM(oomed) do { (oomed) = 1; } while (0)
#define IF_HASH_NONFATAL_OOM(x) x

#else
/* malloc failures result in lost memory, hash tables are unusable */

#ifndef uthash_fatal
#define uthash_fatal(msg) exit(-1)        /* fatal OOM error */
#endif

#define HASH_RECORD_OOM(oomed) uthash_fatal("out of memory")
#define IF_HASH_NONFATAL_OOM(x)

#endif

/*
初始化桶和扩容：
这段代码定义了 UTHash 库中哈希表的初始容量和扩容触发条件的关键参数，直接影响哈希表的内存占用和性能表现

初始桶数量 HASH_INITIAL_NUM_BUCKETS // 初始桶数为 32
作用：哈希表创建时分配的初始桶数组长度。桶是哈希表中存储键值对的容器
设计考量：
内存与性能平衡：32 是一个折中值，避免过小（导致频繁扩容）或过大（浪费内存）
二次幂选择：32 是 2 的 5 次方，便于通过位运算（而非取模）计算索引，提升效率
自定义场景：
若预期存储大量元素（如百万级），可增大初始值（如 256U），减少扩容次数

初始桶数的对数 HASH_INITIAL_NUM_BUCKETS_LOG2 // 因为 2^5 = 32
作用：用于快速计算哈希值到桶索引的映射（替代取模运算）bucket_idx = hashv & ((1 << HASH_INITIAL_NUM_BUCKETS_LOG2) - 1);  // 等价于 hashv % 32
优化意义：位运算 (&) 比取模运算 (%) 更快，尤其在高频操作中显著提升性能
问题：
  取模运算（%）在计算机中本质是除法运算，而除法是 CPU 指令集中较慢的操作。在高频操作（如大规模插入、查询）中，会成为性能瓶颈
  位运算速度极快：& 是 CPU 的原子操作，通常只需 1 个时钟周期，而取模运算可能需要数十个周期，无分支预测开销：位运算不涉及条件判断，避免分支预测失败带来的性能损失
二次幂策略位运算实际应用中的优势：
  动态扩容的高效性：哈希表在扩容时，桶数通常翻倍（new_num_buckets = 2 * old_num_buckets），保持为 2 的幂次方、重新哈希的优化：旧桶中的元素在新桶中的位置只需根据新增的高位比特确定，无需重新计算完整哈希值
  内存对齐友好：2 的幂次方大小的内存分配更易对齐，减少内存碎片，提升缓存利用率
两种哈希策略的对比：
  质数桶数策略：某些哈希表实现使用质数作为桶数（如 31、101），目的是减少哈希冲突
  优点：在哈希函数质量一般时，减少因数据模式导致的聚集
  缺点：无法使用位运算优化索引计算，扩容时需重新选择质数，逻辑复杂
  二次幂策略的适用场景：
  哈希函数质量高（如加密哈希、现代哈希算法），确保哈希值各比特均匀分布
  高频操作场景（如实时系统、游戏引擎），对性能敏感

桶容量扩容阈值 HASH_BKT_CAPACITY_THRESH // 当桶中元素数达到此值时触发扩容
作用：控制哈希表的扩容时机。当任何桶中的元素数量超过此阈值时，哈希表会触发扩容（桶数组长度翻倍）
设计逻辑：
减少哈希冲突：链表过长会降低查找效率（从 O(1) 退化为 O(n)）
平衡空间与时间：阈值越低，扩容越频繁（内存占用高，但查询快）；反之，内存节省但查询可能变慢
调优建议：
高查询性能需求：降低阈值（如 8U），确保桶链表更短
内存敏感场景：提高阈值（如 16U），减少扩容次数

参数关系与扩容机制：
扩容规则：当哈希表中存在至少一个桶的元素数超过 HASH_BKT_CAPACITY_THRESH 时，桶数组长度翻倍（如 32 → 64）
负载因子间接控制：UTHash 通过桶链表长度阈值间接控制负载因子。例如，初始桶数 32，阈值 10，理论最大容纳元素数为 32 * 10 = 320，但实际扩容可能在更早触发
*/
/* initial number of buckets */
#define HASH_INITIAL_NUM_BUCKETS 32U     /* initial number of buckets        */
#define HASH_INITIAL_NUM_BUCKETS_LOG2 5U /* lg2 of initial number of buckets */
#define HASH_BKT_CAPACITY_THRESH 10U     /* expand when bucket count reaches */

/*
偏移量加减计算 elp 元素和 hash 结构体的地址：元素和结构体灵活切换
这两个宏用于 在哈希表元素结构体与其内部的 UT_hash_handle 结构体之间进行地址转换，是 UTHash 库实现泛型哈希表的核心机制。它们通过指针偏移计算，实现了从哈希句柄（UT_hash_handle）到外层元素结构体的反向定位，以及从元素结构体到哈希句柄的正向定位
这两个宏是 UTHash 实现类型安全与高效内存操作的核心，通过编译时已知的偏移量，在元素结构体与哈希句柄之间无缝转换，使哈希表库既能泛化适配任意用户类型，又无需牺牲 C 语言的性能优势

ELMT_FROM_HH(tbl, hhp)：
作用：通过哈希句柄 hhp 的地址，计算出外层元素结构体的地址
参数：tbl：哈希表对象（UT_hash_table），存储哈希表元信息。hhp：指向 UT_hash_handle 的指针（哈希句柄的地址）
计算逻辑：
(char*)(hhp)：将 UT_hash_handle 指针转换为 char* 类型，以便按字节计算偏移
(tbl)->hho：哈希句柄成员在元素结构体中的字节偏移量（通过 offsetof 计算）
减去偏移量：从哈希句柄地址回退 hho 字节，得到外层元素结构体的起始地址，（地址加减获取偏移后的指针）

HH_FROM_ELMT(tbl, elp)：
作用：通过元素结构体地址 elp，计算出其内部 UT_hash_handle 的地址
参数：tbl：哈希表对象。elp：指向外层元素结构体的指针
计算逻辑：
(char*)(elp)：将元素结构体指针转换为 char* 类型
加上偏移量：从元素结构体起始地址前进 hho 字节，得到 UT_hash_handle 成员的地址

关键设计思想
(1) 泛型编程的实现
UTHash 需要支持任意用户定义的结构体作为哈希表元素。通过要求用户在元素结构体中嵌入 UT_hash_handle 成员，并记录该成员的偏移量（hho），库代码可以：
统一操作接口：无论元素结构体如何定义，均通过 UT_hash_handle 进行哈希表操作
类型无关性：无需依赖 C++ 模板或 void 指针强转，保持 C 语言的兼容性
(2) 内存布局与偏移计算
offsetof 的底层依赖：偏移量 hho 通过 offsetof 宏计算
指针运算的安全性：使用 char* 进行指针运算，确保按字节精确偏移（char 类型大小为 1 字节）

应用场景
(1) 遍历哈希表元素
当通过哈希表内部链表遍历时，库代码只能直接访问 UT_hash_handle。需要通过 ELMT_FROM_HH 获取外层元素结构体，才能访问用户数据
(2) 插入/删除元素
用户操作元素时，库需要访问内部的 UT_hash_handle 以维护哈希表结构

注意事项
hho 的初始化：需在哈希表初始化时正确计算，通常通过 HASH_ADD 等宏自动处理
不可滥用指针运算：用户不应手动修改 UT_hash_handle 或偏移量，否则会破坏哈希表结构
多哈希句柄支持：若单个元素需要加入多个哈希表，需定义多个 UT_hash_handle 成员，并分别记录偏移量
*/
/* calculate the element whose hash handle address is hhp */
#define ELMT_FROM_HH(tbl,hhp) ((void*)(((char*)(hhp)) - ((tbl)->hho)))
/* calculate the hash handle from element address elp */
#define HH_FROM_ELMT(tbl,elp) ((UT_hash_handle*)(void*)(((char*)(elp)) + ((tbl)->hho)))

/*
hash表 回滚操作
这个宏HASH_ROLLBACK_BKT用于在哈希表操作中回滚（撤销）某个元素的变更，主要恢复元素所属桶的计数器并重置元素的链表指针
hh：哈希句柄字段名（通常为 hh，用户定义的 UT_hash_handle 成员名称）
head：哈希表头指针（UT_hash_table* 类型）
itemptrhh：待回滚元素的哈希句柄指针（UT_hash_handle* 类型）

？
恢复桶的计数器：通过元素的哈希值计算其所属的桶（HASH_TO_BKT），然后将该桶的计数器（count）加1。这通常用于回滚删除操作，或在哈希表扩容/缩容失败时恢复原桶的计数
重置元素的链表指针：将元素的hh_next和hh_prev指针设为NULL，使其从当前链表中脱离。这可能是因为元素在之前的操作中被移出链表（如删除或迁移），回滚时需要清除残留的指针状态，为后续操作（如重新插入）做准备
*/
#define HASH_ROLLBACK_BKT(hh, head, itemptrhh)                                   \
do {                                                                             \
  struct UT_hash_handle *_hd_hh_item = (itemptrhh);                              \
  unsigned _hd_bkt;                                                              \
  HASH_TO_BKT(_hd_hh_item->hashv, (head)->hh.tbl->num_buckets, _hd_bkt);         \
  (head)->hh.tbl->buckets[_hd_bkt].count++;                                      \
  _hd_hh_item->hh_next = NULL;                                                   \
  _hd_hh_item->hh_prev = NULL;                                                   \
} while (0)

/*
这个宏 HASH_VALUE 是用于计算给定键（key）的哈希值的封装接口

哈希值计算
调用底层的 HASH_FUNCTION，根据键的指针 keyptr 和键的长度 keylen 计算哈希值，并将结果写入 hashv 变量
接口封装
将具体的哈希函数实现（HASH_FUNCTION）包装在 HASH_VALUE 中，提供统一的调用方式。这种分层设计使得更换哈希算法时无需修改上层代码，只需替换 HASH_FUNCTION 的实现

设计意图
可移植性：通过抽象 HASH_FUNCTION，允许在不同平台或场景下灵活切换哈希算法（如 MurmurHash、FNV、CRC 等），而调用方无需感知具体实现
代码复用与维护：统一哈希值计算的调用接口，减少重复代码。当需要优化或修复哈希算法时，只需修改 HASH_FUNCTION 一处
参数标准化：规范化参数顺序（键指针、键长度、输出变量），确保调用的一致性，降低误用风险

注意事项
参数有效性：需确保 keyptr 指向有效数据且 keylen 正确，否则哈希值可能不准确或引发内存问题（如越界访问）
哈希冲突：此宏仅计算哈希值，不处理冲突。实际使用需结合哈希表的冲突解决机制（如链地址法、开放寻址法等）
线程安全：若 HASH_FUNCTION 的实现依赖全局状态（如种子值），需确保多线程环境下的安全性

HASH_VALUE 是哈希表实现中的关键基础设施，通过封装哈希计算过程，实现了算法与接口的解耦，提升了代码的灵活性和可维护性
*/
#define HASH_VALUE(keyptr,keylen,hashv)                                          \
do {                                                                             \
  HASH_FUNCTION(keyptr, keylen, hashv);                                          \
} while (0)

/*
hash 表查找元素
这两个宏 HASH_FIND_BYHASHVALUE 和 HASH_FIND 是 UT 哈希表库中用于查找元素的核心接口，分别用于 已知哈希值 和 需计算哈希值 的场景

HASH_FIND_BYHASHVALUE
功能：在哈希表中通过 已知的哈希值 查找指定键对应的元素，并将结果通过 out 返回。若未找到，out 设为 NULL
实现逻辑：
初始化输出： (out) = NULL; 确保未找到时返回 NULL
哈希表非空检查：if (head) 若哈希表为空，直接跳过查找
计算桶索引：HASH_TO_BKT 通过哈希值 hashval 和当前桶数量 num_buckets，计算键应属的桶索引 _hf_bkt
布隆过滤器快速过滤：if (HASH_BLOOM_TEST((head)->hh.tbl, hashval)) 使用布隆过滤器检查哈希值是否可能存在。若测试失败，直接跳过桶内查找，减少无效遍历
桶内链表遍历：HASH_FIND_IN_BKT 在目标桶的链表中遍历，通过 keyptr 和 keylen 比较键值，找到匹配的元素

HASH_FIND
功能：在哈希表中通过 键的原始数据 查找元素，内部自动计算哈希值后调用 HASH_FIND_BYHASHVALUE
实现逻辑：
初始化输出：(out) = NULL
哈希表非空检查：if (head) 
计算哈希值：HASH_VALUE(keyptr, keylen, _hf_hashv); 调用 HASH_VALUE 宏，根据键的原始数据计算哈希值 _hf_hashv
调用 HASH_FIND_BYHASHVALUE：委托给 HASH_FIND_BYHASHVALUE 完成实际查找

关键设计思想：
分层抽象
HASH_FIND 是对 HASH_FIND_BYHASHVALUE 的封装，隐藏哈希值计算的细节
HASH_FIND_BYHASHVALUE 实现核心逻辑，允许复用哈希值（如插入/删除时已计算过）
性能优化
布隆过滤器：快速排除不可能存在的哈希值，减少桶内遍历
桶定位：直接通过哈希值映射到目标桶，缩小搜索范围
安全性
使用 do { ... } while (0) 包裹宏，避免宏展开导致的语法错误
强制初始化 out 为 NULL，防止野指针

使用场景对比：
HASH_FIND	常规查找，键的哈希值未知	简化调用，自动计算哈希值
HASH_FIND_BYHASHVALUE	已缓存哈希值（如插入/删除后需回查）	避免重复计算哈希值，性能更高

注意事项：
键的唯一性：哈希表假设键唯一，若存在重复键，HASH_FIND 返回第一个匹配项
键比较方式：实际键比较由 HASH_FIND_IN_BKT 实现，需确保与哈希计算时使用的键数据完全一致
布隆过滤器误判：布隆过滤器可能产生假阳性（判断存在但实际不存在），但不会产生假阴性（若返回不存在则一定不存在）
*/
#define HASH_FIND_BYHASHVALUE(hh,head,keyptr,keylen,hashval,out)                 \
do {                                                                             \
  (out) = NULL;                                                                  \
  if (head) {                                                                    \
    unsigned _hf_bkt;                                                            \
    HASH_TO_BKT(hashval, (head)->hh.tbl->num_buckets, _hf_bkt);                  \
    if (HASH_BLOOM_TEST((head)->hh.tbl, hashval)) {                              \
      HASH_FIND_IN_BKT((head)->hh.tbl, hh, (head)->hh.tbl->buckets[ _hf_bkt ], keyptr, keylen, hashval, out); \
    }                                                                            \
  }                                                                              \
} while (0)

#define HASH_FIND(hh,head,keyptr,keylen,out)                                     \
do {                                                                             \
  (out) = NULL;                                                                  \
  if (head) {                                                                    \
    unsigned _hf_hashv;                                                          \
    HASH_VALUE(keyptr, keylen, _hf_hashv);                                       \
    HASH_FIND_BYHASHVALUE(hh, head, keyptr, keylen, _hf_hashv, out);             \
  }                                                                              \
} while (0)

/*
布隆过滤器的加速查询
这段代码是 UT 哈希表库中实现 布隆过滤器（Bloom Filter） 的核心逻辑，用于加速哈希表查询。布隆过滤器通过预判键是否 绝对不存在 于表中，避免无效的链表遍历，从而提升查找效率
这段代码通过条件编译实现了 可选的布隆过滤器支持，核心目标是 加速哈希表查询。其设计平衡了空间与时间的效率，适合对查询性能要求较高且能接受一定误判率的场景。开发者可通过调整 HASH_BLOOM 的值，灵活控制内存占用与误判率的平衡

宏定义与内存管理
HASH_BLOOM：定义 HASH_BLOOM 用于移位 定义布隆过滤器移位的位数，HASH_BLOOM 的值必须小于 unsigned long 类型的位数（如 32 位系统需小于 32），否则可能导致未定义行为（UB）
HASH_BLOOM_BITLEN：布隆过滤器的总位数，定义为 1UL << HASH_BLOOM。例如，若 HASH_BLOOM 设为 16，则位数为 65536 位
HASH_BLOOM_BYTELEN：计算布隆过滤器占用的字节数，公式为 总位数 / 8，并向上取整（不足一字节的部分补一字节）

初始化
HASH_BLOOM_MAKE
初始化布隆过滤器：设置布隆过滤器位数 bloom_nbits
分配内存 bloom_bv，大小为 HASH_BLOOM_BYTELEN
内存分配失败时记录 OOM（Out-Of-Memory）错误
成功分配后，将内存清零，并设置签名 bloom_sig（用于验证结构有效性）

释放
HASH_BLOOM_FREE(tbl) 释放布隆过滤器的内存空间

位操作宏：设置与测试位
HASH_BLOOM_BITSET(bv, idx) 将位数组 bv 的第 idx 位设为 1 ：idx / 8 定位到字节、1U << (idx % 8) 生成位掩码、按位或操作设置对应位
HASH_BLOOM_BITTEST(bv, idx)：测试位数组 bv 的第 idx 位是否为 1、返回 1 表示可能存在，0 表示绝对不存在
0b0000 0000 0000 1000 0100 0000 0000 0000
假设布尔总共 32 位，hashv值 15(必须小于32位)：15/8 = 1、15%8=7
假设布尔总共 32 位，hashv值 20(必须小于32位)：20/8 = 2、20%8=4

哈希值映射：
HASH_BLOOM_ADD(tbl, hashv)：将哈希值 hashv 映射到布隆过滤器的某一位并置位 (hashv) & ((1UL << tbl->bloom_nbits) - 1) 取哈希值的低 bloom_nbits 位作为布隆过滤器的索引
HASH_BLOOM_TEST(tbl, hashv)：测试哈希值 hashv 对应的位是否为 1。若为 0，说明键 绝对不存在 于表中，直接跳过链表遍历

条件编译控制
启用布隆过滤器（#ifdef HASH_BLOOM）：当定义 HASH_BLOOM 时，启用所有布隆过滤器逻辑，包括初始化、内存管理、位操作等
禁用布隆过滤器（#else）：未定义 HASH_BLOOM 时：HASH_BLOOM_MAKE、HASH_BLOOM_FREE、HASH_BLOOM_ADD 为空操作、HASH_BLOOM_TEST 返回 1（即总认为键可能存在，退化为普通哈希表查找）、HASH_BLOOM_BYTELEN 设为 0，不占用额外内存

设计意图与性能影响
优势
快速排除无效查询：布隆过滤器通过位测试，可立即确定键是否 绝对不存在，避免不必要的链表遍历，尤其对大型哈希表效果显著
空间效率：布隆过滤器以极小的内存代价（如 HASH_BLOOM=16 时仅 8KB）换取查询性能提升
权衡
误判率：布隆过滤器可能返回 假阳性（判断键存在但实际不存在），但不会产生 假阴性（若返回不存在则一定不存在）。误判率由位数和哈希函数数量决定，UT 哈希表此处使用单哈希函数，误判率较高，但仍能过滤大部分无效查询。
内存开销：需权衡 HASH_BLOOM 的值：较大的值减少误判率但增加内存占用，较小的值反之
*/
#ifdef HASH_BLOOM
#define HASH_BLOOM_BITLEN (1UL << HASH_BLOOM)
#define HASH_BLOOM_BYTELEN (HASH_BLOOM_BITLEN/8UL) + (((HASH_BLOOM_BITLEN%8UL)!=0UL) ? 1UL : 0UL)
#define HASH_BLOOM_MAKE(tbl,oomed)                                               \
do {                                                                             \
  (tbl)->bloom_nbits = HASH_BLOOM;                                               \
  (tbl)->bloom_bv = (uint8_t*)uthash_malloc(HASH_BLOOM_BYTELEN);                 \
  if (!(tbl)->bloom_bv) {                                                        \
    HASH_RECORD_OOM(oomed);                                                      \
  } else {                                                                       \
    uthash_bzero((tbl)->bloom_bv, HASH_BLOOM_BYTELEN);                           \
    (tbl)->bloom_sig = HASH_BLOOM_SIGNATURE;                                     \
  }                                                                              \
} while (0)

#define HASH_BLOOM_FREE(tbl)                                                     \
do {                                                                             \
  uthash_free((tbl)->bloom_bv, HASH_BLOOM_BYTELEN);                              \
} while (0)

#define HASH_BLOOM_BITSET(bv,idx) (bv[(idx)/8U] |= (1U << ((idx)%8U)))
#define HASH_BLOOM_BITTEST(bv,idx) ((bv[(idx)/8U] & (1U << ((idx)%8U))) != 0)

#define HASH_BLOOM_ADD(tbl,hashv)                                                \
  HASH_BLOOM_BITSET((tbl)->bloom_bv, ((hashv) & (uint32_t)((1UL << (tbl)->bloom_nbits) - 1U)))

#define HASH_BLOOM_TEST(tbl,hashv)                                               \
  HASH_BLOOM_BITTEST((tbl)->bloom_bv, ((hashv) & (uint32_t)((1UL << (tbl)->bloom_nbits) - 1U)))

#else
#define HASH_BLOOM_MAKE(tbl,oomed)
#define HASH_BLOOM_FREE(tbl)
#define HASH_BLOOM_ADD(tbl,hashv)
#define HASH_BLOOM_TEST(tbl,hashv) 1
#define HASH_BLOOM_BYTELEN 0U
#endif

/*
初始化 hash 表：
这个宏 HASH_MAKE_TABLE 是 uthash 库中用于初始化哈希表的核心逻辑，其目的是为哈希表分配内存并初始化关键数据结构

关键设计思想
（1）内存安全与回滚
 - 分配顺序：先分配哈希表结构体，再分配桶，最后布隆过滤器
 - 回滚机制：若后续步骤失败，立即释放已分配的内存，避免泄漏。例如：
 - 若桶分配失败 → 释放哈希表结构体
 - 若布隆过滤器分配失败 → 释放桶和哈希表结构体

（2）性能优化
 预计算字段：
 - log2_num_buckets：将桶数量转换为对数，用位运算 hashv & (num_buckets-1) 代替取模运算
 - hho：通过偏移量快速定位用户数据，避免重复计算
 - 布隆过滤器：通过概率判断减少不必要的链表遍历

（3）数据一致性
 - 签名校验：signature 字段帮助识别内存是否被意外篡改
 - 双向链表：维护 prev/next（应用顺序）和 hh_prev/hh_next（桶内顺序），确保遍历正确性
*/
#define HASH_MAKE_TABLE(hh,head,oomed)                                           \
do {                                                                             \
  (head)->hh.tbl = (UT_hash_table*)uthash_malloc(sizeof(UT_hash_table));         \
  if (!(head)->hh.tbl) {                                                         \
    HASH_RECORD_OOM(oomed);                                                      \
  } else {                                                                       \
    uthash_bzero((head)->hh.tbl, sizeof(UT_hash_table));                         \
    (head)->hh.tbl->tail = &((head)->hh);                                        \
    (head)->hh.tbl->num_buckets = HASH_INITIAL_NUM_BUCKETS;                      \
    (head)->hh.tbl->log2_num_buckets = HASH_INITIAL_NUM_BUCKETS_LOG2;            \
    (head)->hh.tbl->hho = (char*)(&(head)->hh) - (char*)(head);                  \
    (head)->hh.tbl->buckets = (UT_hash_bucket*)uthash_malloc(                    \
        HASH_INITIAL_NUM_BUCKETS * sizeof(struct UT_hash_bucket));               \
    (head)->hh.tbl->signature = HASH_SIGNATURE;                                  \
    if (!(head)->hh.tbl->buckets) {                                              \
      HASH_RECORD_OOM(oomed);                                                    \
      uthash_free((head)->hh.tbl, sizeof(UT_hash_table));                        \
    } else {                                                                     \
      uthash_bzero((head)->hh.tbl->buckets,                                      \
          HASH_INITIAL_NUM_BUCKETS * sizeof(struct UT_hash_bucket));             \
      HASH_BLOOM_MAKE((head)->hh.tbl, oomed);                                    \
      IF_HASH_NONFATAL_OOM(                                                      \
        if (oomed) {                                                             \
          uthash_free((head)->hh.tbl->buckets,                                   \
              HASH_INITIAL_NUM_BUCKETS*sizeof(struct UT_hash_bucket));           \
          uthash_free((head)->hh.tbl, sizeof(UT_hash_table));                    \
        }                                                                        \
      )                                                                          \
    }                                                                            \
  }                                                                              \
} while (0)

/*

*/
#define HASH_REPLACE_BYHASHVALUE_INORDER(hh,head,fieldname,keylen_in,hashval,add,replaced,cmpfcn) \
do {                                                                             \
  (replaced) = NULL;                                                             \
  HASH_FIND_BYHASHVALUE(hh, head, &((add)->fieldname), keylen_in, hashval, replaced); \
  if (replaced) {                                                                \
    HASH_DELETE(hh, head, replaced);                                             \
  }                                                                              \
  HASH_ADD_KEYPTR_BYHASHVALUE_INORDER(hh, head, &((add)->fieldname), keylen_in, hashval, add, cmpfcn); \
} while (0)

#define HASH_REPLACE_BYHASHVALUE(hh,head,fieldname,keylen_in,hashval,add,replaced) \
do {                                                                             \
  (replaced) = NULL;                                                             \
  HASH_FIND_BYHASHVALUE(hh, head, &((add)->fieldname), keylen_in, hashval, replaced); \
  if (replaced) {                                                                \
    HASH_DELETE(hh, head, replaced);                                             \
  }                                                                              \
  HASH_ADD_KEYPTR_BYHASHVALUE(hh, head, &((add)->fieldname), keylen_in, hashval, add); \
} while (0)

#define HASH_REPLACE(hh,head,fieldname,keylen_in,add,replaced)                   \
do {                                                                             \
  unsigned _hr_hashv;                                                            \
  HASH_VALUE(&((add)->fieldname), keylen_in, _hr_hashv);                         \
  HASH_REPLACE_BYHASHVALUE(hh, head, fieldname, keylen_in, _hr_hashv, add, replaced); \
} while (0)

#define HASH_REPLACE_INORDER(hh,head,fieldname,keylen_in,add,replaced,cmpfcn)    \
do {                                                                             \
  unsigned _hr_hashv;                                                            \
  HASH_VALUE(&((add)->fieldname), keylen_in, _hr_hashv);                         \
  HASH_REPLACE_BYHASHVALUE_INORDER(hh, head, fieldname, keylen_in, _hr_hashv, add, replaced, cmpfcn); \
} while (0)

#define HASH_APPEND_LIST(hh, head, add)                                          \
do {                                                                             \
  (add)->hh.next = NULL;                                                         \
  (add)->hh.prev = ELMT_FROM_HH((head)->hh.tbl, (head)->hh.tbl->tail);           \
  (head)->hh.tbl->tail->next = (add);                                            \
  (head)->hh.tbl->tail = &((add)->hh);                                           \
} while (0)

#define HASH_AKBI_INNER_LOOP(hh,head,add,cmpfcn)                                 \
do {                                                                             \
  do {                                                                           \
    if (cmpfcn(DECLTYPE(head)(_hs_iter), add) > 0) {                             \
      break;                                                                     \
    }                                                                            \
  } while ((_hs_iter = HH_FROM_ELMT((head)->hh.tbl, _hs_iter)->next));           \
} while (0)

#ifdef NO_DECLTYPE
#undef HASH_AKBI_INNER_LOOP
#define HASH_AKBI_INNER_LOOP(hh,head,add,cmpfcn)                                 \
do {                                                                             \
  char *_hs_saved_head = (char*)(head);                                          \
  do {                                                                           \
    DECLTYPE_ASSIGN(head, _hs_iter);                                             \
    if (cmpfcn(head, add) > 0) {                                                 \
      DECLTYPE_ASSIGN(head, _hs_saved_head);                                     \
      break;                                                                     \
    }                                                                            \
    DECLTYPE_ASSIGN(head, _hs_saved_head);                                       \
  } while ((_hs_iter = HH_FROM_ELMT((head)->hh.tbl, _hs_iter)->next));           \
} while (0)
#endif

#if HASH_NONFATAL_OOM

#define HASH_ADD_TO_TABLE(hh,head,keyptr,keylen_in,hashval,add,oomed)            \
do {                                                                             \
  if (!(oomed)) {                                                                \
    unsigned _ha_bkt;                                                            \
    (head)->hh.tbl->num_items++;                                                 \
    HASH_TO_BKT(hashval, (head)->hh.tbl->num_buckets, _ha_bkt);                  \
    HASH_ADD_TO_BKT((head)->hh.tbl->buckets[_ha_bkt], hh, &(add)->hh, oomed);    \
    if (oomed) {                                                                 \
      HASH_ROLLBACK_BKT(hh, head, &(add)->hh);                                   \
      HASH_DELETE_HH(hh, head, &(add)->hh);                                      \
      (add)->hh.tbl = NULL;                                                      \
      uthash_nonfatal_oom(add);                                                  \
    } else {                                                                     \
      HASH_BLOOM_ADD((head)->hh.tbl, hashval);                                   \
      HASH_EMIT_KEY(hh, head, keyptr, keylen_in);                                \
    }                                                                            \
  } else {                                                                       \
    (add)->hh.tbl = NULL;                                                        \
    uthash_nonfatal_oom(add);                                                    \
  }                                                                              \
} while (0)

#else

#define HASH_ADD_TO_TABLE(hh,head,keyptr,keylen_in,hashval,add,oomed)            \
do {                                                                             \
  unsigned _ha_bkt;                                                              \
  (head)->hh.tbl->num_items++;                                                   \
  HASH_TO_BKT(hashval, (head)->hh.tbl->num_buckets, _ha_bkt);                    \
  HASH_ADD_TO_BKT((head)->hh.tbl->buckets[_ha_bkt], hh, &(add)->hh, oomed);      \
  HASH_BLOOM_ADD((head)->hh.tbl, hashval);                                       \
  HASH_EMIT_KEY(hh, head, keyptr, keylen_in);                                    \
} while (0)

#endif


#define HASH_ADD_KEYPTR_BYHASHVALUE_INORDER(hh,head,keyptr,keylen_in,hashval,add,cmpfcn) \
do {                                                                             \
  IF_HASH_NONFATAL_OOM( int _ha_oomed = 0; )                                     \
  (add)->hh.hashv = (hashval);                                                   \
  (add)->hh.key = (char*) (keyptr);                                              \
  (add)->hh.keylen = (unsigned) (keylen_in);                                     \
  if (!(head)) {                                                                 \
    (add)->hh.next = NULL;                                                       \
    (add)->hh.prev = NULL;                                                       \
    HASH_MAKE_TABLE(hh, add, _ha_oomed);                                         \
    IF_HASH_NONFATAL_OOM( if (!_ha_oomed) { )                                    \
      (head) = (add);                                                            \
    IF_HASH_NONFATAL_OOM( } )                                                    \
  } else {                                                                       \
    void *_hs_iter = (head);                                                     \
    (add)->hh.tbl = (head)->hh.tbl;                                              \
    HASH_AKBI_INNER_LOOP(hh, head, add, cmpfcn);                                 \
    if (_hs_iter) {                                                              \
      (add)->hh.next = _hs_iter;                                                 \
      if (((add)->hh.prev = HH_FROM_ELMT((head)->hh.tbl, _hs_iter)->prev)) {     \
        HH_FROM_ELMT((head)->hh.tbl, (add)->hh.prev)->next = (add);              \
      } else {                                                                   \
        (head) = (add);                                                          \
      }                                                                          \
      HH_FROM_ELMT((head)->hh.tbl, _hs_iter)->prev = (add);                      \
    } else {                                                                     \
      HASH_APPEND_LIST(hh, head, add);                                           \
    }                                                                            \
  }                                                                              \
  HASH_ADD_TO_TABLE(hh, head, keyptr, keylen_in, hashval, add, _ha_oomed);       \
  HASH_FSCK(hh, head, "HASH_ADD_KEYPTR_BYHASHVALUE_INORDER");                    \
} while (0)

#define HASH_ADD_KEYPTR_INORDER(hh,head,keyptr,keylen_in,add,cmpfcn)             \
do {                                                                             \
  unsigned _hs_hashv;                                                            \
  HASH_VALUE(keyptr, keylen_in, _hs_hashv);                                      \
  HASH_ADD_KEYPTR_BYHASHVALUE_INORDER(hh, head, keyptr, keylen_in, _hs_hashv, add, cmpfcn); \
} while (0)

#define HASH_ADD_BYHASHVALUE_INORDER(hh,head,fieldname,keylen_in,hashval,add,cmpfcn) \
  HASH_ADD_KEYPTR_BYHASHVALUE_INORDER(hh, head, &((add)->fieldname), keylen_in, hashval, add, cmpfcn)

#define HASH_ADD_INORDER(hh,head,fieldname,keylen_in,add,cmpfcn)                 \
  HASH_ADD_KEYPTR_INORDER(hh, head, &((add)->fieldname), keylen_in, add, cmpfcn)

#define HASH_ADD_KEYPTR_BYHASHVALUE(hh,head,keyptr,keylen_in,hashval,add)        \
do {                                                                             \
  IF_HASH_NONFATAL_OOM( int _ha_oomed = 0; )                                     \
  (add)->hh.hashv = (hashval);                                                   \
  (add)->hh.key = (const void*) (keyptr);                                        \
  (add)->hh.keylen = (unsigned) (keylen_in);                                     \
  if (!(head)) {                                                                 \
    (add)->hh.next = NULL;                                                       \
    (add)->hh.prev = NULL;                                                       \
    HASH_MAKE_TABLE(hh, add, _ha_oomed);                                         \
    IF_HASH_NONFATAL_OOM( if (!_ha_oomed) { )                                    \
      (head) = (add);                                                            \
    IF_HASH_NONFATAL_OOM( } )                                                    \
  } else {                                                                       \
    (add)->hh.tbl = (head)->hh.tbl;                                              \
    HASH_APPEND_LIST(hh, head, add);                                             \
  }                                                                              \
  HASH_ADD_TO_TABLE(hh, head, keyptr, keylen_in, hashval, add, _ha_oomed);       \
  HASH_FSCK(hh, head, "HASH_ADD_KEYPTR_BYHASHVALUE");                            \
} while (0)

#define HASH_ADD_KEYPTR(hh,head,keyptr,keylen_in,add)                            \
do {                                                                             \
  unsigned _ha_hashv;                                                            \
  HASH_VALUE(keyptr, keylen_in, _ha_hashv);                                      \
  HASH_ADD_KEYPTR_BYHASHVALUE(hh, head, keyptr, keylen_in, _ha_hashv, add);      \
} while (0)

#define HASH_ADD_BYHASHVALUE(hh,head,fieldname,keylen_in,hashval,add)            \
  HASH_ADD_KEYPTR_BYHASHVALUE(hh, head, &((add)->fieldname), keylen_in, hashval, add)

#define HASH_ADD(hh,head,fieldname,keylen_in,add)                                \
  HASH_ADD_KEYPTR(hh, head, &((add)->fieldname), keylen_in, add)

#define HASH_TO_BKT(hashv,num_bkts,bkt)                                          \
do {                                                                             \
  bkt = ((hashv) & ((num_bkts) - 1U));                                           \
} while (0)

/* delete "delptr" from the hash table.
 * "the usual" patch-up process for the app-order doubly-linked-list.
 * The use of _hd_hh_del below deserves special explanation.
 * These used to be expressed using (delptr) but that led to a bug
 * if someone used the same symbol for the head and deletee, like
 *  HASH_DELETE(hh,users,users);
 * We want that to work, but by changing the head (users) below
 * we were forfeiting our ability to further refer to the deletee (users)
 * in the patch-up process. Solution: use scratch space to
 * copy the deletee pointer, then the latter references are via that
 * scratch pointer rather than through the repointed (users) symbol.
 */
#define HASH_DELETE(hh,head,delptr)                                              \
    HASH_DELETE_HH(hh, head, &(delptr)->hh)

#define HASH_DELETE_HH(hh,head,delptrhh)                                         \
do {                                                                             \
  const struct UT_hash_handle *_hd_hh_del = (delptrhh);                          \
  if ((_hd_hh_del->prev == NULL) && (_hd_hh_del->next == NULL)) {                \
    HASH_BLOOM_FREE((head)->hh.tbl);                                             \
    uthash_free((head)->hh.tbl->buckets,                                         \
                (head)->hh.tbl->num_buckets * sizeof(struct UT_hash_bucket));    \
    uthash_free((head)->hh.tbl, sizeof(UT_hash_table));                          \
    (head) = NULL;                                                               \
  } else {                                                                       \
    unsigned _hd_bkt;                                                            \
    if (_hd_hh_del == (head)->hh.tbl->tail) {                                    \
      (head)->hh.tbl->tail = HH_FROM_ELMT((head)->hh.tbl, _hd_hh_del->prev);     \
    }                                                                            \
    if (_hd_hh_del->prev != NULL) {                                              \
      HH_FROM_ELMT((head)->hh.tbl, _hd_hh_del->prev)->next = _hd_hh_del->next;   \
    } else {                                                                     \
      DECLTYPE_ASSIGN(head, _hd_hh_del->next);                                   \
    }                                                                            \
    if (_hd_hh_del->next != NULL) {                                              \
      HH_FROM_ELMT((head)->hh.tbl, _hd_hh_del->next)->prev = _hd_hh_del->prev;   \
    }                                                                            \
    HASH_TO_BKT(_hd_hh_del->hashv, (head)->hh.tbl->num_buckets, _hd_bkt);        \
    HASH_DEL_IN_BKT((head)->hh.tbl->buckets[_hd_bkt], _hd_hh_del);               \
    (head)->hh.tbl->num_items--;                                                 \
  }                                                                              \
  HASH_FSCK(hh, head, "HASH_DELETE_HH");                                         \
} while (0)

/* convenience forms of HASH_FIND/HASH_ADD/HASH_DEL */
#define HASH_FIND_STR(head,findstr,out)                                          \
do {                                                                             \
    unsigned _uthash_hfstr_keylen = (unsigned)uthash_strlen(findstr);            \
    HASH_FIND(hh, head, findstr, _uthash_hfstr_keylen, out);                     \
} while (0)
#define HASH_ADD_STR(head,strfield,add)                                          \
do {                                                                             \
    unsigned _uthash_hastr_keylen = (unsigned)uthash_strlen((add)->strfield);    \
    HASH_ADD(hh, head, strfield[0], _uthash_hastr_keylen, add);                  \
} while (0)
#define HASH_REPLACE_STR(head,strfield,add,replaced)                             \
do {                                                                             \
    unsigned _uthash_hrstr_keylen = (unsigned)uthash_strlen((add)->strfield);    \
    HASH_REPLACE(hh, head, strfield[0], _uthash_hrstr_keylen, add, replaced);    \
} while (0)
#define HASH_FIND_INT(head,findint,out)                                          \
    HASH_FIND(hh,head,findint,sizeof(int),out)
#define HASH_ADD_INT(head,intfield,add)                                          \
    HASH_ADD(hh,head,intfield,sizeof(int),add)
#define HASH_REPLACE_INT(head,intfield,add,replaced)                             \
    HASH_REPLACE(hh,head,intfield,sizeof(int),add,replaced)
#define HASH_FIND_PTR(head,findptr,out)                                          \
    HASH_FIND(hh,head,findptr,sizeof(void *),out)
#define HASH_ADD_PTR(head,ptrfield,add)                                          \
    HASH_ADD(hh,head,ptrfield,sizeof(void *),add)
#define HASH_REPLACE_PTR(head,ptrfield,add,replaced)                             \
    HASH_REPLACE(hh,head,ptrfield,sizeof(void *),add,replaced)
#define HASH_DEL(head,delptr)                                                    \
    HASH_DELETE(hh,head,delptr)

/* HASH_FSCK checks hash integrity on every add/delete when HASH_DEBUG is defined.
 * This is for uthash developer only; it compiles away if HASH_DEBUG isn't defined.
 */
#ifdef HASH_DEBUG
#include <stdio.h>   /* fprintf, stderr */
#define HASH_OOPS(...) do { fprintf(stderr, __VA_ARGS__); exit(-1); } while (0)
#define HASH_FSCK(hh,head,where)                                                 \
do {                                                                             \
  struct UT_hash_handle *_thh;                                                   \
  if (head) {                                                                    \
    unsigned _bkt_i;                                                             \
    unsigned _count = 0;                                                         \
    char *_prev;                                                                 \
    for (_bkt_i = 0; _bkt_i < (head)->hh.tbl->num_buckets; ++_bkt_i) {           \
      unsigned _bkt_count = 0;                                                   \
      _thh = (head)->hh.tbl->buckets[_bkt_i].hh_head;                            \
      _prev = NULL;                                                              \
      while (_thh) {                                                             \
        if (_prev != (char*)(_thh->hh_prev)) {                                   \
          HASH_OOPS("%s: invalid hh_prev %p, actual %p\n",                       \
              (where), (void*)_thh->hh_prev, (void*)_prev);                      \
        }                                                                        \
        _bkt_count++;                                                            \
        _prev = (char*)(_thh);                                                   \
        _thh = _thh->hh_next;                                                    \
      }                                                                          \
      _count += _bkt_count;                                                      \
      if ((head)->hh.tbl->buckets[_bkt_i].count !=  _bkt_count) {                \
        HASH_OOPS("%s: invalid bucket count %u, actual %u\n",                    \
            (where), (head)->hh.tbl->buckets[_bkt_i].count, _bkt_count);         \
      }                                                                          \
    }                                                                            \
    if (_count != (head)->hh.tbl->num_items) {                                   \
      HASH_OOPS("%s: invalid hh item count %u, actual %u\n",                     \
          (where), (head)->hh.tbl->num_items, _count);                           \
    }                                                                            \
    _count = 0;                                                                  \
    _prev = NULL;                                                                \
    _thh =  &(head)->hh;                                                         \
    while (_thh) {                                                               \
      _count++;                                                                  \
      if (_prev != (char*)_thh->prev) {                                          \
        HASH_OOPS("%s: invalid prev %p, actual %p\n",                            \
            (where), (void*)_thh->prev, (void*)_prev);                           \
      }                                                                          \
      _prev = (char*)ELMT_FROM_HH((head)->hh.tbl, _thh);                         \
      _thh = (_thh->next ? HH_FROM_ELMT((head)->hh.tbl, _thh->next) : NULL);     \
    }                                                                            \
    if (_count != (head)->hh.tbl->num_items) {                                   \
      HASH_OOPS("%s: invalid app item count %u, actual %u\n",                    \
          (where), (head)->hh.tbl->num_items, _count);                           \
    }                                                                            \
  }                                                                              \
} while (0)
#else
#define HASH_FSCK(hh,head,where)
#endif

/* When compiled with -DHASH_EMIT_KEYS, length-prefixed keys are emitted to
 * the descriptor to which this macro is defined for tuning the hash function.
 * The app can #include <unistd.h> to get the prototype for write(2). */
#ifdef HASH_EMIT_KEYS
#define HASH_EMIT_KEY(hh,head,keyptr,fieldlen)                                   \
do {                                                                             \
  unsigned _klen = fieldlen;                                                     \
  write(HASH_EMIT_KEYS, &_klen, sizeof(_klen));                                  \
  write(HASH_EMIT_KEYS, keyptr, (unsigned long)fieldlen);                        \
} while (0)
#else
#define HASH_EMIT_KEY(hh,head,keyptr,fieldlen)
#endif

/* The Bernstein hash function, used in Perl prior to v5.6. Note (x<<5+x)=x*33. */
#define HASH_BER(key,keylen,hashv)                                               \
do {                                                                             \
  unsigned _hb_keylen = (unsigned)keylen;                                        \
  const unsigned char *_hb_key = (const unsigned char*)(key);                    \
  (hashv) = 0;                                                                   \
  while (_hb_keylen-- != 0U) {                                                   \
    (hashv) = (((hashv) << 5) + (hashv)) + *_hb_key++;                           \
  }                                                                              \
} while (0)


/* SAX/FNV/OAT/JEN hash functions are macro variants of those listed at
 * http://eternallyconfuzzled.com/tuts/algorithms/jsw_tut_hashing.aspx
 * (archive link: https://archive.is/Ivcan )
 */
#define HASH_SAX(key,keylen,hashv)                                               \
do {                                                                             \
  unsigned _sx_i;                                                                \
  const unsigned char *_hs_key = (const unsigned char*)(key);                    \
  hashv = 0;                                                                     \
  for (_sx_i=0; _sx_i < keylen; _sx_i++) {                                       \
    hashv ^= (hashv << 5) + (hashv >> 2) + _hs_key[_sx_i];                       \
  }                                                                              \
} while (0)
/* FNV-1a variation */
#define HASH_FNV(key,keylen,hashv)                                               \
do {                                                                             \
  unsigned _fn_i;                                                                \
  const unsigned char *_hf_key = (const unsigned char*)(key);                    \
  (hashv) = 2166136261U;                                                         \
  for (_fn_i=0; _fn_i < keylen; _fn_i++) {                                       \
    hashv = hashv ^ _hf_key[_fn_i];                                              \
    hashv = hashv * 16777619U;                                                   \
  }                                                                              \
} while (0)

#define HASH_OAT(key,keylen,hashv)                                               \
do {                                                                             \
  unsigned _ho_i;                                                                \
  const unsigned char *_ho_key=(const unsigned char*)(key);                      \
  hashv = 0;                                                                     \
  for(_ho_i=0; _ho_i < keylen; _ho_i++) {                                        \
      hashv += _ho_key[_ho_i];                                                   \
      hashv += (hashv << 10);                                                    \
      hashv ^= (hashv >> 6);                                                     \
  }                                                                              \
  hashv += (hashv << 3);                                                         \
  hashv ^= (hashv >> 11);                                                        \
  hashv += (hashv << 15);                                                        \
} while (0)

#define HASH_JEN_MIX(a,b,c)                                                      \
do {                                                                             \
  a -= b; a -= c; a ^= ( c >> 13 );                                              \
  b -= c; b -= a; b ^= ( a << 8 );                                               \
  c -= a; c -= b; c ^= ( b >> 13 );                                              \
  a -= b; a -= c; a ^= ( c >> 12 );                                              \
  b -= c; b -= a; b ^= ( a << 16 );                                              \
  c -= a; c -= b; c ^= ( b >> 5 );                                               \
  a -= b; a -= c; a ^= ( c >> 3 );                                               \
  b -= c; b -= a; b ^= ( a << 10 );                                              \
  c -= a; c -= b; c ^= ( b >> 15 );                                              \
} while (0)

#define HASH_JEN(key,keylen,hashv)                                               \
do {                                                                             \
  unsigned _hj_i,_hj_j,_hj_k;                                                    \
  unsigned const char *_hj_key=(unsigned const char*)(key);                      \
  hashv = 0xfeedbeefu;                                                           \
  _hj_i = _hj_j = 0x9e3779b9u;                                                   \
  _hj_k = (unsigned)(keylen);                                                    \
  while (_hj_k >= 12U) {                                                         \
    _hj_i +=    (_hj_key[0] + ( (unsigned)_hj_key[1] << 8 )                      \
        + ( (unsigned)_hj_key[2] << 16 )                                         \
        + ( (unsigned)_hj_key[3] << 24 ) );                                      \
    _hj_j +=    (_hj_key[4] + ( (unsigned)_hj_key[5] << 8 )                      \
        + ( (unsigned)_hj_key[6] << 16 )                                         \
        + ( (unsigned)_hj_key[7] << 24 ) );                                      \
    hashv += (_hj_key[8] + ( (unsigned)_hj_key[9] << 8 )                         \
        + ( (unsigned)_hj_key[10] << 16 )                                        \
        + ( (unsigned)_hj_key[11] << 24 ) );                                     \
                                                                                 \
     HASH_JEN_MIX(_hj_i, _hj_j, hashv);                                          \
                                                                                 \
     _hj_key += 12;                                                              \
     _hj_k -= 12U;                                                               \
  }                                                                              \
  hashv += (unsigned)(keylen);                                                   \
  switch ( _hj_k ) {                                                             \
    case 11: hashv += ( (unsigned)_hj_key[10] << 24 ); /* FALLTHROUGH */         \
    case 10: hashv += ( (unsigned)_hj_key[9] << 16 );  /* FALLTHROUGH */         \
    case 9:  hashv += ( (unsigned)_hj_key[8] << 8 );   /* FALLTHROUGH */         \
    case 8:  _hj_j += ( (unsigned)_hj_key[7] << 24 );  /* FALLTHROUGH */         \
    case 7:  _hj_j += ( (unsigned)_hj_key[6] << 16 );  /* FALLTHROUGH */         \
    case 6:  _hj_j += ( (unsigned)_hj_key[5] << 8 );   /* FALLTHROUGH */         \
    case 5:  _hj_j += _hj_key[4];                      /* FALLTHROUGH */         \
    case 4:  _hj_i += ( (unsigned)_hj_key[3] << 24 );  /* FALLTHROUGH */         \
    case 3:  _hj_i += ( (unsigned)_hj_key[2] << 16 );  /* FALLTHROUGH */         \
    case 2:  _hj_i += ( (unsigned)_hj_key[1] << 8 );   /* FALLTHROUGH */         \
    case 1:  _hj_i += _hj_key[0];                      /* FALLTHROUGH */         \
    default: ;                                                                   \
  }                                                                              \
  HASH_JEN_MIX(_hj_i, _hj_j, hashv);                                             \
} while (0)

/* The Paul Hsieh hash function */
#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__)             \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)             \
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif
#define HASH_SFH(key,keylen,hashv)                                               \
do {                                                                             \
  unsigned const char *_sfh_key=(unsigned const char*)(key);                     \
  uint32_t _sfh_tmp, _sfh_len = (uint32_t)keylen;                                \
                                                                                 \
  unsigned _sfh_rem = _sfh_len & 3U;                                             \
  _sfh_len >>= 2;                                                                \
  hashv = 0xcafebabeu;                                                           \
                                                                                 \
  /* Main loop */                                                                \
  for (;_sfh_len > 0U; _sfh_len--) {                                             \
    hashv    += get16bits (_sfh_key);                                            \
    _sfh_tmp  = ((uint32_t)(get16bits (_sfh_key+2)) << 11) ^ hashv;              \
    hashv     = (hashv << 16) ^ _sfh_tmp;                                        \
    _sfh_key += 2U*sizeof (uint16_t);                                            \
    hashv    += hashv >> 11;                                                     \
  }                                                                              \
                                                                                 \
  /* Handle end cases */                                                         \
  switch (_sfh_rem) {                                                            \
    case 3: hashv += get16bits (_sfh_key);                                       \
            hashv ^= hashv << 16;                                                \
            hashv ^= (uint32_t)(_sfh_key[sizeof (uint16_t)]) << 18;              \
            hashv += hashv >> 11;                                                \
            break;                                                               \
    case 2: hashv += get16bits (_sfh_key);                                       \
            hashv ^= hashv << 11;                                                \
            hashv += hashv >> 17;                                                \
            break;                                                               \
    case 1: hashv += *_sfh_key;                                                  \
            hashv ^= hashv << 10;                                                \
            hashv += hashv >> 1;                                                 \
            break;                                                               \
    default: ;                                                                   \
  }                                                                              \
                                                                                 \
  /* Force "avalanching" of final 127 bits */                                    \
  hashv ^= hashv << 3;                                                           \
  hashv += hashv >> 5;                                                           \
  hashv ^= hashv << 4;                                                           \
  hashv += hashv >> 17;                                                          \
  hashv ^= hashv << 25;                                                          \
  hashv += hashv >> 6;                                                           \
} while (0)

/* iterate over items in a known bucket to find desired item */
#define HASH_FIND_IN_BKT(tbl,hh,head,keyptr,keylen_in,hashval,out)               \
do {                                                                             \
  if ((head).hh_head != NULL) {                                                  \
    DECLTYPE_ASSIGN(out, ELMT_FROM_HH(tbl, (head).hh_head));                     \
  } else {                                                                       \
    (out) = NULL;                                                                \
  }                                                                              \
  while ((out) != NULL) {                                                        \
    if ((out)->hh.hashv == (hashval) && (out)->hh.keylen == (keylen_in)) {       \
      if (HASH_KEYCMP((out)->hh.key, keyptr, keylen_in) == 0) {                  \
        break;                                                                   \
      }                                                                          \
    }                                                                            \
    if ((out)->hh.hh_next != NULL) {                                             \
      DECLTYPE_ASSIGN(out, ELMT_FROM_HH(tbl, (out)->hh.hh_next));                \
    } else {                                                                     \
      (out) = NULL;                                                              \
    }                                                                            \
  }                                                                              \
} while (0)

/* add an item to a bucket  */
#define HASH_ADD_TO_BKT(head,hh,addhh,oomed)                                     \
do {                                                                             \
  UT_hash_bucket *_ha_head = &(head);                                            \
  _ha_head->count++;                                                             \
  (addhh)->hh_next = _ha_head->hh_head;                                          \
  (addhh)->hh_prev = NULL;                                                       \
  if (_ha_head->hh_head != NULL) {                                               \
    _ha_head->hh_head->hh_prev = (addhh);                                        \
  }                                                                              \
  _ha_head->hh_head = (addhh);                                                   \
  if ((_ha_head->count >= ((_ha_head->expand_mult + 1U) * HASH_BKT_CAPACITY_THRESH)) \
      && !(addhh)->tbl->noexpand) {                                              \
    HASH_EXPAND_BUCKETS(addhh,(addhh)->tbl, oomed);                              \
    IF_HASH_NONFATAL_OOM(                                                        \
      if (oomed) {                                                               \
        HASH_DEL_IN_BKT(head,addhh);                                             \
      }                                                                          \
    )                                                                            \
  }                                                                              \
} while (0)

/* remove an item from a given bucket */
#define HASH_DEL_IN_BKT(head,delhh)                                              \
do {                                                                             \
  UT_hash_bucket *_hd_head = &(head);                                            \
  _hd_head->count--;                                                             \
  if (_hd_head->hh_head == (delhh)) {                                            \
    _hd_head->hh_head = (delhh)->hh_next;                                        \
  }                                                                              \
  if ((delhh)->hh_prev) {                                                        \
    (delhh)->hh_prev->hh_next = (delhh)->hh_next;                                \
  }                                                                              \
  if ((delhh)->hh_next) {                                                        \
    (delhh)->hh_next->hh_prev = (delhh)->hh_prev;                                \
  }                                                                              \
} while (0)

/* Bucket expansion has the effect of doubling the number of buckets
 * and redistributing the items into the new buckets. Ideally the
 * items will distribute more or less evenly into the new buckets
 * (the extent to which this is true is a measure of the quality of
 * the hash function as it applies to the key domain).
 *
 * With the items distributed into more buckets, the chain length
 * (item count) in each bucket is reduced. Thus by expanding buckets
 * the hash keeps a bound on the chain length. This bounded chain
 * length is the essence of how a hash provides constant time lookup.
 *
 * The calculation of tbl->ideal_chain_maxlen below deserves some
 * explanation. First, keep in mind that we're calculating the ideal
 * maximum chain length based on the *new* (doubled) bucket count.
 * In fractions this is just n/b (n=number of items,b=new num buckets).
 * Since the ideal chain length is an integer, we want to calculate
 * ceil(n/b). We don't depend on floating point arithmetic in this
 * hash, so to calculate ceil(n/b) with integers we could write
 *
 *      ceil(n/b) = (n/b) + ((n%b)?1:0)
 *
 * and in fact a previous version of this hash did just that.
 * But now we have improved things a bit by recognizing that b is
 * always a power of two. We keep its base 2 log handy (call it lb),
 * so now we can write this with a bit shift and logical AND:
 *
 *      ceil(n/b) = (n>>lb) + ( (n & (b-1)) ? 1:0)
 *
 */
#define HASH_EXPAND_BUCKETS(hh,tbl,oomed)                                        \
do {                                                                             \
  unsigned _he_bkt;                                                              \
  unsigned _he_bkt_i;                                                            \
  struct UT_hash_handle *_he_thh, *_he_hh_nxt;                                   \
  UT_hash_bucket *_he_new_buckets, *_he_newbkt;                                  \
  _he_new_buckets = (UT_hash_bucket*)uthash_malloc(                              \
           sizeof(struct UT_hash_bucket) * (tbl)->num_buckets * 2U);             \
  if (!_he_new_buckets) {                                                        \
    HASH_RECORD_OOM(oomed);                                                      \
  } else {                                                                       \
    uthash_bzero(_he_new_buckets,                                                \
        sizeof(struct UT_hash_bucket) * (tbl)->num_buckets * 2U);                \
    (tbl)->ideal_chain_maxlen =                                                  \
       ((tbl)->num_items >> ((tbl)->log2_num_buckets+1U)) +                      \
       ((((tbl)->num_items & (((tbl)->num_buckets*2U)-1U)) != 0U) ? 1U : 0U);    \
    (tbl)->nonideal_items = 0;                                                   \
    for (_he_bkt_i = 0; _he_bkt_i < (tbl)->num_buckets; _he_bkt_i++) {           \
      _he_thh = (tbl)->buckets[ _he_bkt_i ].hh_head;                             \
      while (_he_thh != NULL) {                                                  \
        _he_hh_nxt = _he_thh->hh_next;                                           \
        HASH_TO_BKT(_he_thh->hashv, (tbl)->num_buckets * 2U, _he_bkt);           \
        _he_newbkt = &(_he_new_buckets[_he_bkt]);                                \
        if (++(_he_newbkt->count) > (tbl)->ideal_chain_maxlen) {                 \
          (tbl)->nonideal_items++;                                               \
          if (_he_newbkt->count > _he_newbkt->expand_mult * (tbl)->ideal_chain_maxlen) { \
            _he_newbkt->expand_mult++;                                           \
          }                                                                      \
        }                                                                        \
        _he_thh->hh_prev = NULL;                                                 \
        _he_thh->hh_next = _he_newbkt->hh_head;                                  \
        if (_he_newbkt->hh_head != NULL) {                                       \
          _he_newbkt->hh_head->hh_prev = _he_thh;                                \
        }                                                                        \
        _he_newbkt->hh_head = _he_thh;                                           \
        _he_thh = _he_hh_nxt;                                                    \
      }                                                                          \
    }                                                                            \
    uthash_free((tbl)->buckets, (tbl)->num_buckets * sizeof(struct UT_hash_bucket)); \
    (tbl)->num_buckets *= 2U;                                                    \
    (tbl)->log2_num_buckets++;                                                   \
    (tbl)->buckets = _he_new_buckets;                                            \
    (tbl)->ineff_expands = ((tbl)->nonideal_items > ((tbl)->num_items >> 1)) ?   \
        ((tbl)->ineff_expands+1U) : 0U;                                          \
    if ((tbl)->ineff_expands > 1U) {                                             \
      (tbl)->noexpand = 1;                                                       \
      uthash_noexpand_fyi(tbl);                                                  \
    }                                                                            \
    uthash_expand_fyi(tbl);                                                      \
  }                                                                              \
} while (0)


/* This is an adaptation of Simon Tatham's O(n log(n)) mergesort */
/* Note that HASH_SORT assumes the hash handle name to be hh.
 * HASH_SRT was added to allow the hash handle name to be passed in. */
#define HASH_SORT(head,cmpfcn) HASH_SRT(hh,head,cmpfcn)
#define HASH_SRT(hh,head,cmpfcn)                                                 \
do {                                                                             \
  unsigned _hs_i;                                                                \
  unsigned _hs_looping,_hs_nmerges,_hs_insize,_hs_psize,_hs_qsize;               \
  struct UT_hash_handle *_hs_p, *_hs_q, *_hs_e, *_hs_list, *_hs_tail;            \
  if (head != NULL) {                                                            \
    _hs_insize = 1;                                                              \
    _hs_looping = 1;                                                             \
    _hs_list = &((head)->hh);                                                    \
    while (_hs_looping != 0U) {                                                  \
      _hs_p = _hs_list;                                                          \
      _hs_list = NULL;                                                           \
      _hs_tail = NULL;                                                           \
      _hs_nmerges = 0;                                                           \
      while (_hs_p != NULL) {                                                    \
        _hs_nmerges++;                                                           \
        _hs_q = _hs_p;                                                           \
        _hs_psize = 0;                                                           \
        for (_hs_i = 0; _hs_i < _hs_insize; ++_hs_i) {                           \
          _hs_psize++;                                                           \
          _hs_q = ((_hs_q->next != NULL) ?                                       \
            HH_FROM_ELMT((head)->hh.tbl, _hs_q->next) : NULL);                   \
          if (_hs_q == NULL) {                                                   \
            break;                                                               \
          }                                                                      \
        }                                                                        \
        _hs_qsize = _hs_insize;                                                  \
        while ((_hs_psize != 0U) || ((_hs_qsize != 0U) && (_hs_q != NULL))) {    \
          if (_hs_psize == 0U) {                                                 \
            _hs_e = _hs_q;                                                       \
            _hs_q = ((_hs_q->next != NULL) ?                                     \
              HH_FROM_ELMT((head)->hh.tbl, _hs_q->next) : NULL);                 \
            _hs_qsize--;                                                         \
          } else if ((_hs_qsize == 0U) || (_hs_q == NULL)) {                     \
            _hs_e = _hs_p;                                                       \
            if (_hs_p != NULL) {                                                 \
              _hs_p = ((_hs_p->next != NULL) ?                                   \
                HH_FROM_ELMT((head)->hh.tbl, _hs_p->next) : NULL);               \
            }                                                                    \
            _hs_psize--;                                                         \
          } else if ((cmpfcn(                                                    \
                DECLTYPE(head)(ELMT_FROM_HH((head)->hh.tbl, _hs_p)),             \
                DECLTYPE(head)(ELMT_FROM_HH((head)->hh.tbl, _hs_q))              \
                )) <= 0) {                                                       \
            _hs_e = _hs_p;                                                       \
            if (_hs_p != NULL) {                                                 \
              _hs_p = ((_hs_p->next != NULL) ?                                   \
                HH_FROM_ELMT((head)->hh.tbl, _hs_p->next) : NULL);               \
            }                                                                    \
            _hs_psize--;                                                         \
          } else {                                                               \
            _hs_e = _hs_q;                                                       \
            _hs_q = ((_hs_q->next != NULL) ?                                     \
              HH_FROM_ELMT((head)->hh.tbl, _hs_q->next) : NULL);                 \
            _hs_qsize--;                                                         \
          }                                                                      \
          if ( _hs_tail != NULL ) {                                              \
            _hs_tail->next = ((_hs_e != NULL) ?                                  \
              ELMT_FROM_HH((head)->hh.tbl, _hs_e) : NULL);                       \
          } else {                                                               \
            _hs_list = _hs_e;                                                    \
          }                                                                      \
          if (_hs_e != NULL) {                                                   \
            _hs_e->prev = ((_hs_tail != NULL) ?                                  \
              ELMT_FROM_HH((head)->hh.tbl, _hs_tail) : NULL);                    \
          }                                                                      \
          _hs_tail = _hs_e;                                                      \
        }                                                                        \
        _hs_p = _hs_q;                                                           \
      }                                                                          \
      if (_hs_tail != NULL) {                                                    \
        _hs_tail->next = NULL;                                                   \
      }                                                                          \
      if (_hs_nmerges <= 1U) {                                                   \
        _hs_looping = 0;                                                         \
        (head)->hh.tbl->tail = _hs_tail;                                         \
        DECLTYPE_ASSIGN(head, ELMT_FROM_HH((head)->hh.tbl, _hs_list));           \
      }                                                                          \
      _hs_insize *= 2U;                                                          \
    }                                                                            \
    HASH_FSCK(hh, head, "HASH_SRT");                                             \
  }                                                                              \
} while (0)

/* This function selects items from one hash into another hash.
 * The end result is that the selected items have dual presence
 * in both hashes. There is no copy of the items made; rather
 * they are added into the new hash through a secondary hash
 * hash handle that must be present in the structure. */
/*
元素位于多个 hash 表
HASH_SELECT 宏是 uthash 中实现多哈希表索引的核心工具，其设计体现了高效的内存管理和灵活的数据结构操作，适用于需要元素同时存在于多个集合的场景。开发者需注意元素生命周期和错误处理，以确保程序健壮性

核心逻辑：
- 该宏用于将源哈希表中符合条件的元素同时插入到目标哈希表中，使元素在两个哈希表中同时存在，而无需复制数据。其核心逻辑是通过遍历源哈希表，筛选符合条件的元素，并将其以另一个哈希句柄添加到目标哈希表
typedef struct {
    int key;
    int value;
    UT_hash_handle hh_dst;  // 目标哈希表的句柄
    UT_hash_handle hh_src;  // 源哈希表的句柄
} HashStruct;

宏功能：
输入：
- hh_dst：目标哈希表的哈希句柄字段名（如 UT_hash_handle hh2）
- dst：目标哈希表的头指针（可能为空，首次插入时自动初始化）
- hh_src：源哈希表的哈希句柄字段名（如 UT_hash_handle hh1）
- src：源哈希表的头指针
- cond：条件函数，返回 true 的元素会被选中
输出：
- 目标哈希表 dst 包含所有满足 cond 的元素，且这些元素仍保留在源哈希表中

代码逻辑解析：
（1）初始化：
 - _src_bkt, _dst_bkt：遍历源和目标哈希表的桶索引
 - _last_elt, _last_elt_hh：维护目标哈希表应用顺序链表的尾元素
 - _dst_hho：计算目标哈希句柄在用户结构体中的偏移量
（2）遍历源哈希表：
 - 双重循环：外层遍历所有桶，内层遍历每个桶中的元素
 - ELMT_FROM_HH：通过源哈希句柄获取用户结构体指针
 - 条件检查：调用 cond(_elt) 判断元素是否需要添加到目标表
（3）处理目标哈希表：
 - 链接元素到目标表：句柄操作：直接使用源元素的键和哈希值，避免重新计算。链表维护：按插入顺序维护目标表的 prev/next 指针
 - 初始化目标表（首次插入时）：DECLTYPE_ASSIGN：确保 dst 类型与元素类型一致。HASH_MAKE_TABLE：分配并初始化目标哈希表的管理结构
 - 插入到目标哈希桶：哈希桶定位：根据哈希值计算目标桶索引。桶链表插入：将元素句柄添加到目标桶的链表头部
 - 错误回滚：OOM 处理：若添加元素到桶时内存不足，回滚操作并跳过当前元素

关键设计思想
- 双重存在机制：同一元素，多哈希表：通过不同哈希句柄（如 hh1 和 hh2）让元素同时存在于多个哈希表，适用于多索引场景。无数据拷贝：直接引用源元素，节省内存和复制开销
- 内存安全：OOM 回滚：在内存分配失败时，回滚桶操作并清理句柄，避免数据结构损坏。布隆过滤器更新：HASH_BLOOM_ADD 确保新元素能被快速查找
- 性能优化：哈希值复用：直接使用源哈希表的 hashv，避免重复计算。链表维护：按插入顺序维护目标表，支持顺序遍历

注意事项
- 元素生命周期：元素必须在其所属的所有哈希表中被正确删除，否则会导致内存泄漏或悬空指针
- 线程安全：遍历过程中若其他线程修改源哈希表，可能导致未定义行为
- 键一致性：源表和目标表的键（key/keylen）必须一致，否则查找会失败
- 错误处理：若启用了 HASH_NONFATAL_OOM，需实现 uthash_nonfatal_oom 函数处理内存不足
*/
#define HASH_SELECT(hh_dst, dst, hh_src, src, cond)                              \
do {                                                                             \
  unsigned _src_bkt, _dst_bkt;                                                   \
  void *_last_elt = NULL, *_elt;                                                 \
  UT_hash_handle *_src_hh, *_dst_hh, *_last_elt_hh=NULL;                         \
  ptrdiff_t _dst_hho = ((char*)(&(dst)->hh_dst) - (char*)(dst));                 \
  if ((src) != NULL) {                                                           \
    for (_src_bkt=0; _src_bkt < (src)->hh_src.tbl->num_buckets; _src_bkt++) {    \
      for (_src_hh = (src)->hh_src.tbl->buckets[_src_bkt].hh_head;               \
        _src_hh != NULL;                                                         \
        _src_hh = _src_hh->hh_next) {                                            \
        _elt = ELMT_FROM_HH((src)->hh_src.tbl, _src_hh);                         \
        if (cond(_elt)) {                                                        \
          IF_HASH_NONFATAL_OOM( int _hs_oomed = 0; )                             \
          _dst_hh = (UT_hash_handle*)(void*)(((char*)_elt) + _dst_hho);          \
          _dst_hh->key = _src_hh->key;                                           \
          _dst_hh->keylen = _src_hh->keylen;                                     \
          _dst_hh->hashv = _src_hh->hashv;                                       \
          _dst_hh->prev = _last_elt;                                             \
          _dst_hh->next = NULL;                                                  \
          if (_last_elt_hh != NULL) {                                            \
            _last_elt_hh->next = _elt;                                           \
          }                                                                      \
          if ((dst) == NULL) {                                                   \
            DECLTYPE_ASSIGN(dst, _elt);                                          \
            HASH_MAKE_TABLE(hh_dst, dst, _hs_oomed);                             \
            IF_HASH_NONFATAL_OOM(                                                \
              if (_hs_oomed) {                                                   \
                uthash_nonfatal_oom(_elt);                                       \
                (dst) = NULL;                                                    \
                continue;                                                        \
              }                                                                  \
            )                                                                    \
          } else {                                                               \
            _dst_hh->tbl = (dst)->hh_dst.tbl;                                    \
          }                                                                      \
          HASH_TO_BKT(_dst_hh->hashv, _dst_hh->tbl->num_buckets, _dst_bkt);      \
          HASH_ADD_TO_BKT(_dst_hh->tbl->buckets[_dst_bkt], hh_dst, _dst_hh, _hs_oomed); \
          (dst)->hh_dst.tbl->num_items++;                                        \
          IF_HASH_NONFATAL_OOM(                                                  \
            if (_hs_oomed) {                                                     \
              HASH_ROLLBACK_BKT(hh_dst, dst, _dst_hh);                           \
              HASH_DELETE_HH(hh_dst, dst, _dst_hh);                              \
              _dst_hh->tbl = NULL;                                               \
              uthash_nonfatal_oom(_elt);                                         \
              continue;                                                          \
            }                                                                    \
          )                                                                      \
          HASH_BLOOM_ADD(_dst_hh->tbl, _dst_hh->hashv);                          \
          _last_elt = _elt;                                                      \
          _last_elt_hh = _dst_hh;                                                \
        }                                                                        \
      }                                                                          \
    }                                                                            \
  }                                                                              \
  HASH_FSCK(hh_dst, dst, "HASH_SELECT");                                         \
} while (0)

/*
HASH_CLEAR 宏用于安全释放哈希表的管理结构内存，并将头指针置空

HASH_CLEAR 宏释放哈希表内部结构占用的内存，包括：
- 布隆过滤器（若启用）：通过 HASH_BLOOM_FREE 释放位数组内存
- 哈希桶数组：释放所有桶的内存
- 哈希表管理结构（UT_hash_table）：释放元数据内存
- 置空头指针：将 head 设为 NULL，避免悬空指针，悬空指针（Dangling Pointer）是指向已被释放或无效内存的指针。访问悬空指针会导致未定义行为（程序崩溃、数据损坏、安全漏洞等），是 C/C++ 中常见的内存管理错误之一

注意事项
- 仅释放管理结构：不会自动释放用户元素的内存，需先遍历删除元素并手动释放，否则导致内存泄漏
- 头指针有效性：调用后 head 被设为 NULL，避免后续误操作
- 依赖自定义内存管理：若使用自定义的 uthash_malloc 和 uthash_free，需确保其正确性（如线程安全、内存池管理）
- 布隆过滤器兼容性：若未启用 HASH_BLOOM，HASH_BLOOM_FREE 可能为空操作
- 重复调用安全：多次调用 HASH_CLEAR 是安全的（因 head 被置空后不再执行操作）

HASH_CLEAR 是释放哈希表管理资源的关键操作，需与元素内存释放配合使用，确保：
- 正确顺序：先释放元素，再清除表结构
- 内存安全：避免悬空指针和内存泄漏
- 适配场景：结合自定义内存管理和布隆过滤器配置
*/
#define HASH_CLEAR(hh,head)                                                      \
do {                                                                             \
  if ((head) != NULL) {                                                          \
    HASH_BLOOM_FREE((head)->hh.tbl);                                             \
    uthash_free((head)->hh.tbl->buckets,                                         \
                (head)->hh.tbl->num_buckets*sizeof(struct UT_hash_bucket));      \
    uthash_free((head)->hh.tbl, sizeof(UT_hash_table));                          \
    (head) = NULL;                                                               \
  }                                                                              \
} while (0)

/*
HASH_OVERHEAD 用于计算哈希表自身管理结构（非用户数据）的内存开销，帮助开发者量化哈希表的内存占用情况

HASH_OVERHEAD 用于计算哈希表内部管理结构的内存开销总和，包括：
- 所有元素的哈希句柄（UT_hash_handle）
- 哈希桶数组（UT_hash_bucket 数组）
- 哈希表管理结构（UT_hash_table）
- 布隆过滤器（Bloom Filter，若启用）
- 返回值：总开销（单位：字节），类型为 size_t

关键设计思想
- 量化内存开销：目的帮助开发者评估哈希表自身的管理成本，尤其在内存敏感场景（如嵌入式系统）中优化内存使用
- 条件编译支持：布隆过滤器若未启用 HASH_BLOOM，HASH_BLOOM_BYTELEN 被定义为 0，不增加开销
- 空指针安全：若 head 为 NULL（哈希表未初始化或为空），直接返回 0，避免访问非法内存

使用场景
- 场景 1：内存优化分析
- 场景 2：动态扩容监控

注意事项
不包含用户数据：此宏仅计算管理结构的内存，用户结构体的内存需单独统计（如 num_items * sizeof(UserEntry)）
布隆过滤器开销：若启用 HASH_BLOOM，HASH_BLOOM_BYTELEN 通常为 (1 << bloom_nbits) / 8 字节，默认 bloom_nbits 可能为 10（占用 128 字节）
动态变化：哈希表扩容或元素增减时，num_buckets 和 num_items 会变化，总开销随之改变

总结
HASH_OVERHEAD 宏是 uthash 库中重要的内存分析工具，通过量化哈希表内部管理结构的开销，帮助开发者：
- 优化内存使用，尤其是在资源受限的环境中
- 监控哈希表扩容/缩容时的内存变化
- 对比不同哈希表实现的效率（如负载因子、布隆过滤器的影响）
*/
#define HASH_OVERHEAD(hh,head)                                                   \
 (((head) != NULL) ? (                                                           \
 (size_t)(((head)->hh.tbl->num_items   * sizeof(UT_hash_handle))   +             \
          ((head)->hh.tbl->num_buckets * sizeof(UT_hash_bucket))   +             \
           sizeof(UT_hash_table)                                   +             \
           (HASH_BLOOM_BYTELEN))) : 0U)

/*
遍历：
该宏用于遍历 uthash 哈希表中的所有元素，支持不同编译环境下的类型安全处理

（1）宏功能：
 - HASH_ITER 用于按应用顺序（插入顺序）遍历哈希表中的所有元素。其核心逻辑是通过 for 循环逐个访问链表中的元素，直到结束

（2）参数：
 - hh：哈希句柄字段名（如 UT_hash_handle hh）
 - head：哈希表头指针（指向第一个元素）
 - el：当前元素指针
 - tmp：临时变量，用于保存下一个元素的指针

（3）宏实现解析：HASH_ITER 的实现分为两种场景，通过 NO_DECLTYPE 条件编译选择
 - 支持 DECLTYPE 的版本
 DECLTYPE 的作用：自动推断 el 的类型，确保 tmp 的类型与 el 一致（如 struct User*）
 遍历逻辑：
 初始化：el 设为 head，tmp 设为 head->hh.next（下一个元素）
 条件检查：当 el 不为 NULL 时执行循环体
 迭代更新：将 el 更新为 tmp，tmp 更新为下一个元素的 hh.next
 - 不支持 DECLTYPE 的版本
 绕过类型检查：
 将 tmp 的地址强制转换为 char**，再通过 char* 赋值
 通过指针操作绕过编译器类型检查，确保 tmp 能接收 void* 类型的 hh.next
 适用场景：
 编译器不支持类型推断时，手动处理指针类型转换

（4）关键设计思想
 - 类型安全与兼容性
 DECLTYPE 版本：通过类型推断确保 tmp 的类型与 el 一致，避免类型错误
 NO_DECLTYPE 版本：使用底层指针操作强制赋值，兼容不支持类型推断的旧编译器
 - 遍历机制
 应用顺序链表：hh.next 字段直接指向下一个用户结构体的指针（而非句柄地址），无需计算偏移量
 字段定义：UT_hash_handle 中的 next 字段类型为 void*，实际存储用户结构体的指针
 - 空指针保护
 条件检查：(head != NULL) 和 (tmp != NULL) 确保空指针不会导致崩溃

（5）底层依赖
 - UT_hash_handle 结构体：next 字段：直接指向下一个用户结构体，而非句柄或中间结构
 - HASH_ADD 的隐含逻辑：当调用 HASH_ADD 插入元素时，uthash 自动维护 prev 和 next 指针，形成应用顺序的双向链表

（6）总结
 HASH_ITER 宏通过灵活的指针操作和条件编译，实现了对哈希表元素的安全遍历：
 - 类型安全：利用 DECLTYPE 或底层指针转换确保类型正确
 - 高效性：直接访问链表字段，复杂度为 O(n)
 - 兼容性：支持新旧编译器环境
*/
#ifdef NO_DECLTYPE
#define HASH_ITER(hh,head,el,tmp)                                                \
for(((el)=(head)), ((*(char**)(&(tmp)))=(char*)((head!=NULL)?(head)->hh.next:NULL)); \
  (el) != NULL; ((el)=(tmp)), ((*(char**)(&(tmp)))=(char*)((tmp!=NULL)?(tmp)->hh.next:NULL)))
#else
#define HASH_ITER(hh,head,el,tmp)                                                \
for(((el)=(head)), ((tmp)=DECLTYPE(el)((head!=NULL)?(head)->hh.next:NULL));      \
  (el) != NULL; ((el)=(tmp)), ((tmp)=DECLTYPE(el)((tmp!=NULL)?(tmp)->hh.next:NULL)))
#endif

/* obtain a count of items in the hash */
/*
获取 hash 表元素数
这两个宏 HASH_COUNT 和 HASH_CNT 是 uthash 库中用于获取哈希表中元素数量的工具，其核心作用是安全地读取哈希表当前存储的元素总数

（1）宏定义解析：
 - HASH_COUNT(head)：用户直接调用的宏，内部转发到 HASH_CNT，默认哈希句柄字段名为 hh（用户结构体中必须包含 UT_hash_handle hh;）
 - HASH_CNT(hh, head)：实际执行逻辑的宏，参数 hh：用户结构体中哈希句柄的字段名（可自定义，但通常为 hh）。参数 head：指向哈希表头节点的指针

（2）设计思想：
 安全性：
 - 空指针保护：直接检查 head 是否为 NULL，避免访问非法内存
 - 类型安全：返回 0U（无符号整型），与 num_items 的无符号类型一致，防止隐式类型转换问题
 高效性
 - O(1) 时间复杂度：直接访问 num_items 字段，无需遍历链表或桶
 - 无副作用：宏展开为简单表达式，不涉及函数调用开销
 灵活性
 - 支持自定义句柄名：通过 HASH_CNT 的 hh 参数，允许用户结构体中的哈希句柄字段使用其他名称（如 my_hh），此时需显式调用

（3）注意事项
 - 哈希表未初始化：若未调用 HASH_MAKE_TABLE 初始化哈希表，head->hh.tbl 可能为 NULL，此时访问 num_items 会导致崩溃。但 HASH_CNT 的 head != NULL 检查已隐含此情况（未初始化的 head 通常为 NULL）
 - 线程安全：多线程环境中需确保对哈希表的操作（如插入、删除）是原子的，否则 num_items 可能不准确
 - 返回值类型：宏返回 unsigned int 类型（0U），需根据使用场景决定是否强制转换为 size_t 或其他类型
*/
#define HASH_COUNT(head) HASH_CNT(hh,head)
#define HASH_CNT(hh,head) ((head != NULL)?((head)->hh.tbl->num_items):0U)

/*
UT_hash_bucket 是 uthash 库中管理哈希冲突的核心结构，其设计特点包括：
 - 高效冲突处理：通过链表管理相同哈希值的元素
 - 动态扩展控制：利用 expand_mult 灵活调整扩展阈值
 - 性能权衡：平衡查询与插入操作的开销
*/
typedef struct UT_hash_bucket {
   // 作用：指向哈希冲突链表的头节点（UT_hash_handle 类型）说明：哈希表使用链地址法解决冲突，每个桶对应一个链表，链表中存储哈希值相同的元素。新元素通常插入链表头部（时间复杂度 O(1)）
   struct UT_hash_handle *hh_head;
   // 作用：当前桶中元素的数量（链表长度）说明：用于监控哈希表的负载均衡性。触发扩容的阈值判断：当 count > HASH_BKT_CAPACITY_THRESH * (expand_mult + 1) 时，可能触发桶扩展
   unsigned count;

   /* expand_mult is normally set to 0. In this situation, the max chain length
    * threshold is enforced at its default value, HASH_BKT_CAPACITY_THRESH. (If
    * the bucket's chain exceeds this length, bucket expansion is triggered).
    * However, setting expand_mult to a non-zero value delays bucket expansion
    * (that would be triggered by additions to this particular bucket)
    * until its chain length reaches a *multiple* of HASH_BKT_CAPACITY_THRESH.
    * (The multiplier is simply expand_mult+1). The whole idea of this
    * multiplier is to reduce bucket expansions, since they are expensive, in
    * situations where we know that a particular bucket tends to be overused.
    * It is better to let its chain length grow to a longer yet-still-bounded
    * value, than to do an O(n) bucket expansion too often.
    */
   // 作用：桶扩展的倍数控制，用于延迟桶扩展，减少频繁扩容的开销。
   // 说明：
   // 默认行为：expand_mult = 0 时，当 count > HASH_BKT_CAPACITY_THRESH（如 10）时触发扩展 
   // 自定义行为：若 expand_mult > 0，扩展阈值变为 HASH_BKT_CAPACITY_THRESH * (expand_mult + 1)
   // 设计动机：某些桶可能因哈希函数特性被频繁使用，通过提高其扩展阈值，减少扩容次数，牺牲少量查询性能换取更高的插入效率
   unsigned expand_mult;

} UT_hash_bucket;

/* random signature used only to find hash tables in external analysis */
/*
唯一签名：
这两个宏定义是 uthash 库中用于标识哈希表和布隆过滤器（Bloom Filter）的唯一签名，目的是在调试或内存分析时快速识别数据结构

HASH_SIGNATURE：
作用：
- 哈希表的唯一标识符，存储在 UT_hash_table 结构体的 signature 字段中
用途：
- 调试：在内存转储（如 GDB 或 Valgrind）中，通过搜索此魔数（Magic Number）快速定位哈希表结构
- 数据校验：在运行时验证指针是否指向合法的哈希表（避免野指针或内存损坏）
- 外部工具：供内存分析工具（如调试器、泄漏检测工具）识别哈希表结构

HASH_BLOOM_SIGNATURE：
作用：
- 布隆过滤器的唯一标识符，存储在 UT_hash_table 的 bloom_sig 字段中（仅在启用布隆过滤器时存在）
用途：
-布隆过滤器校验：确认布隆过滤器是否已正确初始化
-内存分析：帮助工具定位哈希表中布隆过滤器的内存位置

设计意图
唯一性：选用的值（如 0xa0111fe1u 和 0xb12220f2u）是人为设计的 32 位无符号整数，确保在内存中不易与其他数据冲突
防御性编程：在关键操作前校验签名，防止因内存越界、重复释放或野指针导致的未定义行为
调试友好性：开发人员或工具可通过内存扫描快速定位哈希表，分析其状态（如桶数量、元素数量）

实际应用场景
调试崩溃问题：当程序崩溃时，若怀疑哈希表损坏，可在内存中搜索 0xa0111fe1u 找到所有哈希表实例，检查其内部状态
检测内存泄漏：内存分析工具可通过签名识别未释放的哈希表，统计泄漏数量
安全校验：库函数在操作哈希表前检查 signature，若不符则立即终止，避免操作非法内存

注意事项
非功能性字段：签名不影响哈希表的核心逻辑，仅用于调试和校验
编译条件：HASH_BLOOM_SIGNATURE 仅在启用布隆过滤器（通过 HASH_BLOOM 宏）时生效
释放内存时的处理：某些实现可能在释放哈希表时将签名置零，明确标记内存无效
*/
#define HASH_SIGNATURE 0xa0111fe1u
#define HASH_BLOOM_SIGNATURE 0xb12220f2u

/*
UT_hash_table 是 uthash 库的核心结构，负责管理哈希表的内存、扩容策略和性能优化。其设计体现了以下特点：
 - 高效性：通过位运算、布隆过滤器和负载监控提升操作速度
 - 鲁棒性：处理内存分配失败、无效扩容等边界情况
 - 灵活性：支持用户自定义结构体，通过 hho 偏移量实现泛型操作
*/
typedef struct UT_hash_table {
   // 作用：指向哈希桶数组的指针，哈希表通过哈希函数将键映射到桶的索引。每个桶（UT_hash_bucket）是一个链表的头节点，用于处理哈希冲突（链地址法）
   UT_hash_bucket *buckets;
   // 作用：哈希表的桶数量和它的对数，num_buckets：当前哈希桶的总数（如 32、64）。log2_num_buckets：以2为底的桶数量的对数（如 log2(32) = 5）。优化计算：哈希值取模时，可用位运算 hash_value & (num_buckets - 1) 代替 % 运算（需 num_buckets 是2的幂）
   unsigned num_buckets, log2_num_buckets;
   // 作用：哈希表中当前存储的元素总数。说明：用于触发扩容：当元素数量超过 num_buckets * 理想链长度 时，哈希表可能扩容
   unsigned num_items;
   // 作用：指向哈希表中最后一个元素的句柄（UT_hash_handle），说明：维护插入顺序：支持快速在尾部追加元素（如 HASH_ADD 的 APPEND 模式），初始化为头节点的句柄，随着元素插入逐步更新
   struct UT_hash_handle *tail; /* tail hh in app order, for fast append    */
   // 作用：哈希句柄 UT_hash_handle 在用户结构体中的字节偏移量，说明：通过元素指针计算哈希句柄的位置，用于从元素指针快速定位哈希句柄，支持链表操作
   ptrdiff_t hho; /* hash handle offset (byte pos of hash handle in element */

   /* in an ideal situation (all buckets used equally), no bucket would have
    * more than ceil(#items/#buckets) items. that's the ideal chain length. */
   // 作用：理想情况下，单个桶的最大链长度，说明：计算公式：ceil(num_items / num_buckets)。用于衡量哈希表的负载均衡性。若实际链长度超过此值，说明哈希分布不均匀  
   unsigned ideal_chain_maxlen;

   /* nonideal_items is the number of items in the hash whose chain position
    * exceeds the ideal chain maxlen. these items pay the penalty for an uneven
    * hash distribution; reaching them in a chain traversal takes >ideal steps */
   // 作用：当前哈希表中链长度超过 ideal_chain_maxlen 的元素数量。说明：非理想元素越多，哈希表的查找效率越低（需要遍历更长的链表）
   unsigned nonideal_items;

   /* ineffective expands occur when a bucket doubling was performed, but
    * afterward, more than half the items in the hash had nonideal chain
    * positions. If this happens on two consecutive expansions we inhibit any
    * further expansion, as it's not helping; this happens when the hash
    * function isn't a good fit for the key domain. When expansion is inhibited
    * the hash will still work, albeit no longer in constant time. */
   // 作用：记录无效扩容次数和禁止扩容标志。说明：ineff_expands：连续扩容后，若超过半数元素仍处于非理想链中，此值递增。noexpand：当 ineff_expands >= 2 时，禁止进一步扩容，避免因哈希函数不匹配导致性能恶化 
   unsigned ineff_expands, noexpand;

   // 作用：哈希表的唯一签名标识。说明：用于调试或外部工具校验哈希表结构的合法性（如检测内存损坏）
   uint32_t signature; /* used only to find hash tables in external analysis */

   /*
   布隆过滤器相关字段（HASH_BLOOM）
   作用：
   - 加速键的存在性检查
   说明：
   - 布隆过滤器：一种概率型数据结构，通过多个哈希函数将键映射到位向量中
   - 查询优化：若布隆过滤器报告键不存在，则无需遍历桶链表
   - 内存开销：bloom_nbits 越大，误判率越低，但内存占用越高
   */ 
#ifdef HASH_BLOOM
   // 布隆过滤器签名
   uint32_t bloom_sig; /* used only to test bloom exists in external analysis */
   // 布隆过滤器位向量
   uint8_t *bloom_bv;
   // 布隆过滤器的位数（如10表示2^10位)
   uint8_t bloom_nbits;
#endif

} UT_hash_table;

/*
UT_hash_handle 是 uthash 库的核心纽带，负责：
 - 将用户数据绑定到哈希表
 - 维护元素在应用顺序和哈希冲突链表中的位置
 - 存储键信息和哈希值，支持快速查找和动态扩容
*/
typedef struct UT_hash_handle {
   // 作用：指向该元素所属的哈希表（UT_hash_table），说明：每个元素通过此字段关联到哈希表，支持多哈希表场景（如一个元素可同时加入多个哈希表）。在删除或移动元素时，用于验证操作的正确性
   struct UT_hash_table *tbl;
   // 作用：按应用顺序（插入顺序或用户定义的顺序）维护元素的双向链表，说明：prev：指向按应用顺序排列的前一个元素的用户结构体指针，next：指向按应用顺序排列的下一个元素的用户结构体指针，应用场景：按插入顺序遍历元素（如实现 LRU 缓存），快速在尾部追加元素（HASH_ADD 的 APPEND 模式）
   void *prev;                       /* prev element in app order      */
   void *next;                       /* next element in app order      */
   // 作用：按桶内顺序维护哈希冲突链表的双向链表，说明：hh_prev：指向同一哈希桶中前一个元素的 UT_hash_handle，hh_next：指向同一哈希桶中下一个元素的 UT_hash_handle，应用场景：处理哈希冲突时，遍历桶内链表查找元素，在桶内快速插入或删除元素
   struct UT_hash_handle *hh_prev;   /* previous hh in bucket order    */
   struct UT_hash_handle *hh_next;   /* next hh in bucket order        */
   // 作用：存储用户定义的键及其长度，说明：key：指向用户结构体中键的指针（如 &entry->id），keylen：键的长度（字节数），用于哈希计算和键比较，设计动机：支持任意类型的键（整数、字符串、结构体等），避免键值拷贝，直接引用用户结构体中的键
   const void *key;                  /* ptr to enclosing struct's key  */
   unsigned keylen;                  /* enclosing struct's key len     */
   // 作用：缓存键的哈希值（Hash Value），说明：在元素插入时，通过哈希函数（如 HASH_FUNCTION）计算并存储此值，避免重复计算哈希值，提升扩容、查找等操作的效率
   unsigned hashv;                   /* result of hash-fcn(key)        */
} UT_hash_handle;

#endif /* UTHASH_H */
