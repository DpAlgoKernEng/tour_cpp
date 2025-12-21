#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
/*
一、哈希函数的作用：
（1）核心功能：将任意长度的输入数据（如字符串、二进制流）映射为一个固定长度的整数值
（2）关键特性：确定性：相同输入始终得到相同哈希值、雪崩效应：输入微小变化导致输出剧烈变化、均匀分布：理想情况下，哈希值应均匀分布在值域内

二、哈希算法种类：哈希算法种类繁多，根据用途和特性可以分为以下几类，以下是常见哈希算法的分类及典型代表：
加密型哈希算法：用于数据完整性校验、数字签名、密码存储等安全场景，特点是 抗碰撞性强 且 不可逆
（1）SHA 系列（安全哈希算法）：
   - SHA-1：160 位输出，已不推荐使用（存在碰撞漏洞）
   - SHA-2：包括 SHA-224、SHA-256、SHA-384、SHA-512，广泛用于 TLS/SSL、数字证书等
   - SHA-3：基于 Keccak 算法（海绵结构），抗量子计算攻击，如 SHA3-256
（2）MD5：128 位输出，曾广泛用于文件校验，但因碰撞漏洞被弃用
（3）BLAKE2：比 SHA-3 更快，适用于高性能场景（如区块链）
（4）RIPEMD：如 RIPEMD-160（比特币地址生成中曾使用）

非加密型哈希算法：用于数据结构（如哈希表）、快速查找、负载均衡等，**速度优先**，不强调安全性
（1）MurmurHash：高性能，低碰撞率，如 MurmurHash3（分布式系统常用）
（2）xxHash：极速哈希，适用于实时数据处理（如游戏、流媒体）
（3）FNV（Fowler–Noll–Vo）：简单易实现，用于哈希表、DNS 缓存
（4）CityHash / FarmHash：Google 开发，针对长字符串优化
（5）Jenkins Hash：如 `lookup3`，用于数据库索引

一致性哈希（Consistent Hashing）专为分布式系统设计，解决节点动态增删时的数据迁移问题  
（1）特点：哈希环结构，数据分布均匀，减少节点变化时的数据迁移量  
（2）应用：Redis 集群、CDN、分布式缓存（如 Memcached）

特殊用途哈希
（1）CRC（循环冗余校验）：如 CRC32、CRC64，用于网络传输、存储设备的数据校验
（2）GeoHash：将地理坐标编码为字符串，用于地理位置索引（如地图搜索）
（3）SimHash：生成文本指纹，用于相似内容检测（如网页去重）
（4）MinHash：估算数据相似性，适用于大规模数据集去重（如推荐系统）

哈希算法选择指南
（1）密码存储/数字签名：SHA-256、SHA-3、BLAKE2、Argon2（KDF）
（2）文件完整性校验：SHA-256、SHA3-256
（3）哈希表、缓存键计算：MurmurHash3、xxHash、FNV
（4）分布式系统负载均衡：一致性哈希（如 Ketama 算法）
（5）实时数据流处理：xxHash、CityHash
（6）地理位置编码：GeoHash、S2 Geometry

注意事项
（1）避免已破解算法：如 MD5、SHA-1 不再用于安全场景
（2）盐值（Salt）：密码存储需结合随机盐值（如 PBKDF2、bcrypt）
（3）性能权衡：加密型哈希慢但安全，非加密型快但易碰撞
（4）标准化：优先选择行业标准算法（如 NIST 推荐的 SHA-2/SHA-3）
*/

/*
（1）Times33：Times33哈希算法是一种高效且简单的非加密哈希函数，特别适用于字符串哈希表和缓存键。其核心思想是通过迭代计算，将每个字符的ASCII值累加到哈希值中
 算法特点
 - 效率高：乘法和位运算速度快，适合高频使用场景
 - 低碰撞率：33作为奇质数，分散性好，减少不同字符串的哈希冲突
 - 实现简单：代码简洁，易于集成到各类应用中
 算法步骤
 - 初始化：哈希值初始为0
 - 迭代计算：遍历字符串的每个字符，执行 hash = hash * 33 + 当前字符的ASCII值
 - 优化处理：通常结合位运算（如 hash = (hash << 5) + hash 代替乘法）和取模操作（如32位整数溢出），确保计算高效且结果在合理范围内 
 适用场景
 - 内存数据库的键值哈希（如Redis内部字典）
 - 高频字符串处理的缓存键生成
*/
/**
* @brief             times33 hash 算法
* @param   key       接收一个字符串或char数组或char指针
* @return            返回生成的 hash 值
*
* @note              Revision History
*/
unsigned int times33Hash(const char* key) {
    // 初始哈希值为0
    unsigned int hashv = 0;
    // 遍历字符串直到空字符'\0'
    while(*key){
        // hashv += hashv * 33 + (unsigned char)(*key);
        // 优化：使用 hash = (hash << 5) + hash 代替乘法
        hashv += (hashv << 5) + hashv + (unsigned char)(*key);
        // 移动指针到下一个字符
        key++;
    }
    return hashv;
}

/*
（2）MurmurHash：MurmurHash 是一种高性能的非加密哈希函数，由 Austin Appleby 设计，广泛用于分布式系统、哈希表、布隆过滤器等场景，常见的版本有 MurmurHash2 和 MurmurHash3
 算法特点
 - 高速度：针对现代 CPU 优化，处理速度极快，单次循环处理 4 字节，适合现代 CPU 流水线优化，仅需单次遍历输入数据，无额外内存开销
 - 低碰撞率：哈希分布均匀，抗碰撞性强，通过多重混合操作，哈希值分布接近均匀
 - 可调种子：允许自定义种子值，生成不同哈希序列，允许通过不同种子生成不同的哈希序列，适用于多实例场景
 - 支持多种数据类型：适用于字符串、二进制数据等
 算法步骤
 - 初始化：设置哈希值 h 和种子 seed，将种子与一个常量混合（例如 0xcc9e2d51），避免全零种子的问题
 - 分块处理：将输入数据按 4字节（32位） 分块处理。对每个块进行位运算和混合操作（如乘法、异或、位移），确保数据充分"混淆"
 - 尾部处理：处理剩余不足 4 字节的数据，避免遗漏
 - 最终混合：对哈希值进行多次混合操作（如异或、位移、乘法），消除残留的规律性
 适用场景
 - 分布式系统的一致性哈希分片（如Cassandra）
 - 高速哈希表（如C++ STL的std::unordered_map优化）

 MurmurHash3 的含义：
 （1）Murmur：该哈希算法系列的命名，无特殊技术含义
 （2）Hash3：表示这是 MurmurHash 算法的第3个主要版本（Austin Appleby 设计的一系列改进版本）
 （3）非加密性：专注于快速计算和低碰撞率，而非安全性（不适用于密码学场景）
 
 32位 的含义：
 （1）输出长度：生成的哈希值是一个 32 位无符号整数（即 uint32_t，范围 0x00000000 ~ 0xFFFFFFFF，十进制 0 ~ 4,294,967,295）
 （2）固定长度：无论输入数据多长，最终输出始终为 4 字节（例如字符串 "hello" 和文件内容均被压缩为 32 位值）
 （3）碰撞概率：理论上，32 位哈希的碰撞概率约为（约 42 亿分之一），实际应用中需权衡性能与碰撞风险
 
 注意事项：
 （1）字节序问题：直接强制转换 uint8_t* 为 uint32_t* 在小端/大端平台可能表现不同
 （2）内存对齐：未对齐内存访问在某些平台（如 ARM）可能导致性能下降或崩溃
 （3）种子选择：若需避免哈希碰撞攻击，种子应随机化（如使用随机数生成器）
*/
/**
* @brief             MurmurHash3 32位哈希算法
* @return  uint32_t  Return Description
*
* @note              Revision History
*/
uint32_t murmurHash3_32(const void *key, size_t len, uint32_t seed) {
    // 初始化与常量定义: 设计目的：通过乘法、位移和异或操作，确保输入数据的每一位都能影响最终哈希值
    // 混合常数1（增强随机性），0xcc9e2d51 和 0x1b873593 是精心选择的质数，通过乘法将数据扩散到整个 32 位空间
    const uint32_t c1 = 0xcc9e2d51;
    // 混合常数2
    const uint32_t c2 = 0x1b873593;
    // 循环左移位数
    const int r1 = 15, r2 = 13;
    // 分块混合参数
    const uint32_t m = 5, n = 0xe6546b64;
    // 初始哈希值 = 种子
    uint32_t hash = seed;
    
    // 核心操作：循环左移：打破数据的局部性，增强雪崩效应，乘法与异或：将数据块的每一位扩散到哈希值中
    // 转为 uint8_t 是 8位无符号整数 的标准类型（范围 0x00 ~ 0xFF），确保每个字节被当作非负数处理，二进制安全：直接操作原始字节，适用于任意数据类型（字符串、结构体、二进制文件等）
    const uint8_t* data = (const uint8_t*)(key);
    // 计算 包含多少个 4 字节
    const int nblocks = len / 4;
    // 转为 4 字节指针，进行指针++ 平移
    const uint32_t* blocks = (const uint32_t*)(data); 
    // 循环处理每一个 4 字节块
    for (size_t i = 0; i < nblocks; i++) {
        // 读取处理一个 4 字节块
        uint32_t k = blocks[i];
        // 乘法混淆数据
        k *= c1;               
        // 循环左移15位，原理：将 k 左移 r1 位，同时将溢出的高位补到低位，形成循环效果。作用：增强位级扩散，避免简单的位移导致信息丢失           
        k *= (k << r1) | (k >> (32 - r1));
        // 再次乘法混淆
        k *= c2;

        // 异或操作更新哈希值，将混淆后的块与当前哈希值混合
        hash ^= k; 
        // 哈希值循环左移 13 位
        hash = (hash << r2) | (hash >> (32 - r2));
        // 混合哈希值：线性混合进一步扩散数据
        hash = hash * m + n;
    }
    
    // 意义：确保不足 4字节 的数据仍参与哈希计算，避免数据遗漏
    // 处理尾部不足4字节的数据，指针平移到最后 nblocks * 4 的位置
    const uint8_t* tail = (const uint8_t*)(data + nblocks * 4);
    uint32_t k1 = 0;
    // 取余运算判断剩余字节数，技巧：利用 switch 的 case 穿透（fall-through）特性，简洁处理1-3字节的尾部数据
    switch (len & 3) {                // len % 4
        case 3: k1 ^= tail[2] << 16;  // 处理第3字节
        case 2: k1 ^= tail[1] << 8;   // 处理第2字节
        case 1: k1 ^= tail[0];        // 处理第1字节
                k1 *= c1;             // 与分块相同的混淆操作
                k1 *= (k1 << r1) | (k1 >> (32 - r1));
                k1 *= c2;
                hash ^= k1;           // 合并到哈希值
    }
    
    // 最终混合（消除规律性）目的：进一步打乱哈希值，确保即使输入有规律，输出也呈现均匀分布
    // 混合数据长度
    hash ^= len;
    // 高16位与低16位混合
    hash ^= (hash >> 16); 
    // 乘法增强随机性  
    hash *= 0x85ebca6b;
    // 高13位与低13位混合
    hash ^= (hash >> 13);
    // 乘法增强随机性  
    hash *= 0xc2b2ae35;
    // 高16位与低16位混合
    hash ^= (hash >> 16);

    return hash;
} 

/*
（3）SHA-256：基于 Merkle-Damgård 结构，通过 64 轮非线性变换（逻辑函数、模加）处理 512 位块
 算法特点：
 - 结构：基于 Merkle-Damgård 迭代结构（分块压缩）
 - 高安全性：目前无已知有效碰撞攻击，抗预映像性强
 - 标准化：NIST认证，广泛集成于安全协议（如TLS、SSL）
 核心步骤：
 - 消息填充：将输入数据填充至长度模 512 余 448，附加 64 位长度信息。示例：输入 "hello" → 填充为 "hello" + 0x80 + ... + 长度位
 - 分块处理：每 512 位块扩展为 64 个 32 位字（W[0..63]）。初始化 8 个 32 位工作变量（a, b, c, d, e, f, g, h）
 - 压缩函数（64 轮迭代）：每轮使用逻辑函数（如 Ch(x,y,z), Maj(x,y,z)）和预定义常数 K[i]，更新工作变量
 - 最终哈希：合并所有块的中间哈希值，输出 256 位结果
 应用场景：
 - 数字签名（如 SSL/TLS 证书）
 - 区块链（比特币的区块哈希）
 - 密码存储（配合盐值）
 算法实现：
 - SHA-256：使用 OpenSSL 库实现，成熟且高效 https://github.com/openssl/openssl.git
*/
/**
* @brief             计算输入数据的 SHA-256 哈希值
* @param   data      data：输入数据的指针（const void*），支持任意二进制数据（字符串、字节流等）
* @param   len       len：输入数据的长度（size_t），单位为字节
* @param   hash      hash：输出缓冲区（unsigned char[]），固定为 SHA256_DIGEST_LENGTH（即 32 字节）
*
* @note              调用 OpenSSL 的 SHA256 函数，将输入数据转换为 const unsigned char* 类型，确保二进制兼容性，计算结果直接写入 hash 数组
*/
#ifdef OPENSSL
#include <openssl/sha.h>
void sha256Hash(const void* data, size_t len, unsigned char hash[SHA256_DIGEST_LENGTH]) {
    SHA256((const unsigned char*)data, len, hash);
}
#endif

/*
（4）SHA-3
 算法特点：
 - 结构：基于 海绵结构（Sponge Construction），吸收数据后"挤压"出哈希值
 - 抗量子攻击：无算术操作，结构简单，对量子计算攻击的潜在抵抗力更强
 - 抗碰撞性：设计目标为 2^128 的碰撞抵抗（与 SHA-256 同级）
 - 硬件友好：纯位操作，适合嵌入式设备和FPGA加速
 - 灵活性：支持可变输出长度（224/256/384/512位）
 核心步骤：
 - 消息填充：使用 10*1 填充模式（如 0x06 + 0x80），确保长度对齐到 r 位（速率参数）
 - 吸收阶段：将输入分块（每块 r 位，如 SHA3-256 的 r = 1088 位），每块与状态异或后，应用 Keccak-f 置换
 - Keccak-f 置换：包含 24 轮非线性操作（θ, ρ, π, χ, ι 步骤），打乱状态位
 - 挤压阶段：从状态中提取哈希值（如 SHA3-256 提取 256 位）
 应用场景：
 - 替代 SHA-2 的新一代安全协议（如未来 TLS 版本）
 - 轻量级设备（硬件效率高）
 - 需要抗侧信道攻击的场景（无算术操作）
 算法实现：
 - SHA-3：基于轻量级开源库实现，适合研究和新协议开发 https://github.com/mjosaarinen/tiny_sha3.git
*/
/**
* @brief             计算输入数据的 SHA3-256 哈希值
* @param   input     input：输入数据的指针（const void*），支持任意二进制数据
* @param   len       len：输入数据的长度（size_t）
* @param   output    output：输出缓冲区（uint8_t[32]），固定为 32 字节
*
* @note              调用第三方库 tiny_sha3 的 sha3 函数，参数 256 表示生成 256 位（32 字节）的哈希值
*/
#ifdef SHA3
#include "../../tiny_lib/tiny_sha3/sha3.h"
void sha3_256Hash(const void* input, size_t len, void* output) {
    sha3(input, len, output, 256); 
}
#endif

/*
（5）SAX
 算法特点：
 - 简单性：通过位移（Shift）、加法（Add）、异或（XOR）组合操作实现
 - 快速性：无复杂运算，适合高频次简单哈希场景
 - 低碰撞率（小数据）：对短字符串或小数据有较好的分布性
 - 非加密性：不适用于安全场景，仅用于数据结构优化
 - 局限性：高碰撞率：输入数据量大时，碰撞概率显著增加、非均匀分布：哈希值分布依赖输入模式，可能不均匀、安全性差：易通过反向工程或碰撞攻击破解、长度敏感：长字符串可能导致哈希值溢出，需额外取模控制范围
 核心步骤：
 - 初始化哈希值：通常为一个非零初始值（如 0 或 5381）
 - 遍历输入字符：逐个处理每个字符
 - 组合操作：Shift（位移）：将当前哈希值左移若干位、Add（加法）：将字符的 ASCII 值加到哈希值、XOR（异或）：将哈希值与位移后的值异或
 - 返回哈希值：最终结果取模或直接输出
 使用场景：
 - 小型哈希表：用于快速键值查找（如内存缓存）
 - 数据分片：将数据按哈希值分配到不同存储块
 - 简单校验：检测数据是否被意外修改（非安全性场景）
*/
/**
* @brief             SAX 哈希算法
* @param   key       Param Description
* @return  uint32_t  Return Description
*
* @note              SAX 哈希并非标准哈希算法的通用名称，可能为某种自定义的简化哈希算法。以下内容基于常见的 Shift-Add-XOR 哈希逻辑进行假设性解析
*/
uint32_t saxHash(const char* key) {
    // hash 初始值
    uint32_t hash = 0;

    // 逐个处理每个字符
    while (*key) {
        // Shift-Add-XOR 组合操作 （类似 Times33）
        hash = (hash << 5) + hash + (uint8_t)(*key);
        // XOR 右移后的值
        hash ^= (hash >> 3);
        key++;
    }

    return hash;
} 

/*
（6）FNV
 算法特点：
 - 简单性：仅需乘法和异或操作，代码实现简洁
 - 高效性：单次遍历数据，时间复杂度为 O(n)，适合高频次计算
 - 低碰撞率：对短字符串和小型数据有良好的分布性
 - 非加密性：不适用于安全场景，易被逆向或碰撞攻击
 - 可调参数：支持自定义初始哈希值（offset_basis）和质数（FNV_prime）
 - 字节序处理：FNV 按字节处理数据，天然兼容大端序和小端序系统
 - 长数据碰撞：长数据可能导致碰撞率上升，建议结合其他算法（如 MurmurHash）混合使用
 - 避免安全场景：不可用于密码存储或数字签名（需使用 SHA 系列等加密哈希）
 FNV-1a 核心步骤：
 - 初始化哈希值：根据位宽选择初始值（如 32 位初始值为 0x811C9DC5）
 - 遍历输入字节：逐个处理输入数据的每个字节（uint8_t）
 - 异或与乘法：对当前哈希值和字节值进行异或，再乘以质数（如 32 位质数为 0x01000193）
 - 返回结果：最终哈希值为 32 位无符号整数
 使用场景：
 - 哈希表键值：快速生成键的哈希值（如 Python 字典内部优化）
 - 数据分片：将数据按哈希值分配到不同存储节点（如分布式系统）
 - 校验和：轻量级数据完整性校验（非安全性场景）
 - 网络协议：快速生成报文哈希标识（如 HTTP 请求去重）
*/
/**
* @brief             FNV 哈希算法：FNV-1a 32位哈希函数
* @param   key       Param Description
* @return  uint32_t  Return Description
*
* @note              FNV (Fowler-Noll-Vo) 是一种简单高效的非加密哈希算法，广泛应用于快速哈希计算场景，如哈希表、数据分片和校验和生成。其核心思想是通过质数乘法和异或操作实现数据混合
*/
uint32_t fnvHash(const char* key, size_t len) {
    // 初始值
    const uint32_t FNV_offset_basis = 0x811C9DC5; 
    // 质数 
    const uint32_t FNV_prime = 0x01000193;
    // 初始值 hash
    uint32_t hash = FNV_offset_basis;

    // 转换为字节流        
    const uint8_t* bytes = (const uint8_t*)key;
    // 逐个处理每个字节
    for(size_t i = 0; i < len; i++) {
        // 异或当前字节
        hash ^= bytes[i]; 
        // 乘以质数  
        hash *= FNV_prime;  
    }

    return hash;
}

/*
（7）OAT（OAAT）
 算法特点：
 - 简单性：仅需加法、位移和异或操作，代码简短
 - 快速性：单次遍历数据，时间复杂度 O(n)，适合高频计算
 - 低碰撞率：对短字符串和小数据有良好的分布性
 - 非加密性：不适用于安全场景，仅用于数据结构优化
 - 可逆性：无加密强度，不可用于密码学
 - 字节序处理：OAT 按字节处理数据，天然兼容大端序和小端序系统
 - 长数据碰撞：长数据可能导致碰撞率上升，建议结合其他算法（如 MurmurHash）混合使用
 - 避免安全场景：不可用于密码存储或数字签名（需使用 SHA 系列等加密哈希） 
 核心步骤：以 32位版本 为例
 - 初始化哈希值：初始值为 0
 - 逐字节处理：遍历输入数据的每个字节（uint8_t），依次进行以下操作 加法：将当前字节值加到哈希值、左移：哈希值左移 10 位（hash += hash << 10）、异或：哈希值右移 6 位后异或原值（hash ^= hash >> 6）
 - 最终混合：循环结束后，对哈希值进行额外处理以消除尾部残留规律 hash += hash << 3; hash ^= hash >> 11; hash += hash << 15;
 - 返回结果：输出 32 位无符号整数
 使用场景：
 - 哈希表键值：快速生成键的哈希值（如内存数据库索引）
 - 数据分片：将数据按哈希值分布到不同存储节点
 - 缓存键生成：高频次字符串的快速哈希计算（如 URL 缓存）
 - 轻量级校验：数据完整性校验（非安全性场景）
*/
/**
* @brief             OAT 哈希算法
* @param   key       Param Description
* @param   len       Param Description
* @return  uint32_t  Return Description
*
* @note              OAT（OAAT）哈希是一种高效、简洁的非加密哈希算法，适用于需要快速计算哈希值的场景（如内存数据库、缓存系统）。其实现简单且对短数据表现良好，但在长数据或安全敏感场景中需谨慎使用。实际开发中，可根据需求选择更优化的算法（如 MurmurHash3）或加密哈希（如 SHA-256）
*/
uint32_t oatHash(const char* key, size_t len) {
    // 初始值 hash
    uint32_t hash = 0;
    
    // 转换为字节流处理
    const uint8_t* bytes = (const uint8_t*)key;
    // 逐个处理每个字节
    for(size_t i = 0; i < len; i++){
        // 加法
        hash += bytes[i];
        // 左移 10 位并累加
        hash += hash << 10;
        // 右移 6 位异或
        hash ^= hash >> 6;
    }
    
    // 最终混合
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;

    return hash;
}

/*
（8）JEN（Jenkins Lookup3）
 算法特点：
 - 高效性：针对现代 CPU 优化，单次处理 12 字节（分块处理）
 - 低碰撞率：严格测试验证，适用于大规模数据集
 - 可逆混合：通过多轮位移、加减、异或操作实现数据充分混合
 - 非加密性：设计目标为快速哈希表键值计算，不适用于安全场景
 - 种子支持：允许自定义种子（seed），生成不同哈希序列
 - 安全性：不适用于密码学场景，需选择 SHA-3 或 BLAKE3 等加密哈希
 核心步骤：
 - 初始化：初始化三个 32 位变量（a, b, c），其中 c 作为种子（或混合结果）
 - 分块处理（每 12 字节）：将输入数据按 12 字节 分块，每块转换为三个 uint32_t 值（小端序），并进行混合
 - 混合函数（mix）：通过多轮位操作打乱数据
 - 处理尾部数据：对剩余不足 12 字节的数据逐个处理
 - 最终哈希值：返回 c 作为主哈希值（可根据需求组合 a, b, c）
 使用场景：
 - 哈希表：快速生成键的哈希值（如 Linux 内核中的 jhash）
 - 数据库索引：为大规模数据生成唯一标识
 - 数据分片：分布式系统中按哈希值分配数据
 - 网络协议：快速生成报文摘要（如去重检测）
*/
/**
* @brief             Jenkins Lookup3 哈希算法
* @param   key       Param Description
* @param   len       Param Description
* @param   seed      Param Description
* @return  uint32_t  Return Description
*
* @note              Jenkins 哈希算法（"jen" 指 Bob Jenkins 设计的 Lookup3 或类似算法）Bob Jenkins 是著名哈希算法设计者，其开发的 Lookup3 和 SpookyHash 等算法以高效和低碰撞率著称
*/
// 32位循环位移
#define rol32(val, shift) (((val) << (shift)) | ((val) >> (32 - (shift))))
// mix 混合函数
#define mix(a, b, c) do {               \
    (a) -= (c); (a) ^= rol32(c, 4); (c) += (b);   \
    (b) -= (a); (b) ^= rol32(a, 6); (a) += (c);   \
    (c) -= (b); (c) ^= rol32(b, 8); (b) += (a);   \
    (a) -= (c); (a) ^= rol32(c, 16); (c) += (b);  \
    (b) -= (a); (b) ^= rol32(a, 19); (a) += (c);  \
    (c) -= (b); (c) ^= rol32(b, 4); (b) += (a);   \
} while(0)

uint32_t jenHash(const char* key, size_t len, uint32_t seed) {
    // 初始值
    uint32_t a, b, c;
    a = b = 0xdeadbeef;  
    c = seed;

    // 转为 二进制字节流 1字节 8位
    const uint8_t* data = (const uint8_t*)(key);
    // 12 字节 一次循环
    while(len >= 12) {
        // 每块转换为三个 uint32_t 值（小端序），并进行混合
        uint32_t k1, k2, k3;
        // 数据拷贝到 k1, k2, k3，使用 memcpy 代替直接类型转换（*(uint32_t*)data）以避免未对齐访问
        memcpy(&k1, data, 4);
        memcpy(&k2, data + 4, 4);
        memcpy(&k3, data + 8, 4);
        // 累加到 a, b, c上
        a += k1; b += k2; c += k3;
        // 调用混合函数
        mix(a, b, c);
        // 指针偏移 12 个字节
        data += 12;
        // 已处理长度减少 12 个字节
        len -= 12;
    }

    // 处理尾部字节
    // 记录剩余长度
    c += len;
    switch (len) {
        case 11: c += ((uint32_t)data[10]) << 24;
        case 10: c += ((uint32_t)data[9]) << 16;   
        case 9:  c += ((uint32_t)data[8]) << 8;    
        case 8:  b += ((uint32_t)data[7]) << 24;   
        case 7:  b += ((uint32_t)data[6]) << 16;   
        case 6:  b += ((uint32_t)data[5]) << 8;    
        case 5:  b += data[4];                     
        case 4:  a += ((uint32_t)data[3]) << 24;   
        case 3:  a += ((uint32_t)data[2]) << 16;   
        case 2:  a += ((uint32_t)data[1]) << 8;    
        case 1:  a += data[0];                     
        case 0:  break;         
    }
    // 再次调用混合函数
    mix(a, b, c);

    return c;
}

int main(void) {

    // 示例1：计算字符串 "abcd" 的哈希值
    const char* str = "abcd";
    unsigned int hash1 = times33Hash(str);
    printf("times33 Hash of \"%s\": %u\n", str, hash1);
    // 示例2：验证碰撞案例（理论上可能）
    const char* str2 = "A";
    const char* str3 = "\0A";                            // 包含空字符的字符串
    unsigned int hash2 = times33Hash(str2);
    unsigned int hash3 = times33Hash(str3);
    printf("times33 Hash of \"%s\": %u\n", str2, hash2);  // 输出 65
    printf("times33 Hash of \"\\0A\": %u\n", hash3);      // 输出 65

    // 示例1：计算字符串 "hello" 的哈希值
    const char* murstr1 = "hello";
    uint32_t murhash1 = murmurHash3_32((const void*)murstr1, strlen(murstr1), 0);
    printf("murmur Hash of \"%s\": 0x%08x\n", murstr1, murhash1); // 输出 0x248bfa47
    // 示例2：验证相同输入不同种子结果不同
    uint32_t murhash2 = murmurHash3_32((const void*)murstr1, strlen(murstr1), 42);
    printf("murmur Hash of \"%s\" (seed=42): 0x%08x\n", murstr1, murhash2); // 输出 0x5e928f0f
    // 示例3：验证不同输入结果不同
    const char* murstr2 = "world";
    uint32_t murhash3 = murmurHash3_32((const void*)murstr2, strlen(murstr2), 0);
    printf("murmur Hash of \"%s\": 0x%08x\n", murstr2, murhash3); // 输出 0x5ba3c492

    // 示例1：计算字符串 "hello" 的哈希值
    const char* saxstr = "hello";
    uint32_t saxhash = saxHash(saxstr);
    printf("sax hash of \"%s\": 0x%08x\n", saxstr, saxhash);

    // 示例1：计算字符串 "hello" 的哈希值
    const char* fnvstr = "hello";
    uint32_t fnvhash = fnvHash(fnvstr, strlen(fnvstr));
    printf("fnv-1a hash of \"%s\": 0x%08x\n", fnvstr, fnvhash);

    // 示例1：计算字符串 "hello" 的哈希值
    const char* oatstr = "hello";
    uint32_t oathash = oatHash(oatstr, strlen(oatstr));
    printf("oat hash of \"%s\": 0x%08x\n", oatstr, oathash);

    // 示例1：计算字符串 "hello" 的哈希值
    const char* jenstr = "hello";
    uint32_t jenhash = jenHash(jenstr, strlen(jenstr), 0);
    printf("jenkins hash of \"%s\": 0x%08x\n", jenstr, jenhash);

    return 0;
}