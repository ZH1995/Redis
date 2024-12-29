/* zmalloc - total amount of allocated memory aware version of malloc()
 *
 * Copyright (c) 2006-2009, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <string.h>
// static：这个关键字表示 used_memory 变量的作用域仅限于定义它的文件,其他文件无法访问这个变量
// 用于跟踪程序中动态分配的内存总量
static size_t used_memory = 0;

// 分配动态内存并跟踪分配的内存量
void *zmalloc(size_t size) {
    // 在内存块开头存储一个size_t类型的变量来记录内存块的大小
    // 所以共需申请size+sizeof(size_t)的内存
    void *ptr = malloc(size+sizeof(size_t));
    // 将实际大小size写入内存块的开头
    // ptr指向分配内存的指针，强制转换为size_t指针，然后写入size
    *((size_t*)ptr) = size;
    // 更新已使用内存字节数
    used_memory += size+sizeof(size_t);
    // 跳过存储大小信息的部分，返回一个指向实际可用内存的指针
    return (char*)ptr+sizeof(size_t);
}

// 重新分配动态内存并跟踪分配的内存量
void *zrealloc(void *ptr, size_t size) {
    void *realptr;
    size_t oldsize;
    void *newptr;
    // 如果传入的指针ptr为NULL，那么直接调用zmalloc函数
    if (ptr == NULL) return zmalloc(size);
    // 获取存储在内存块开头的实际指针realptr,即指向分配内存的起始位置
    realptr = (char*)ptr-sizeof(size_t);
    // 从realptr获取内存块的实际大小
    oldsize = *((size_t*)realptr);
    // 调用realloc函数来重新分配内存，新分配的内存大小为size+sizeof(size_t)
    // 如果realloc失败，则返回NULL
    newptr = realloc(realptr,size+sizeof(size_t));
    if (!newptr) return NULL;
    // 将新分配的内存大小写入内存块的开头
    *((size_t*)newptr) = size;
    // 更新已使用内存字节数，注意oldsize是用户请求的内存大小，不包含size_t信息
    // 所以此处只需加上用户新请求的内存大小即可
    used_memory -= oldsize;
    used_memory += size;
    // 返回新分配的内存块的指针，跳过存储大小信息的部分
    return (char*)newptr+sizeof(size_t);
}

// 释放动态内存并跟踪释放的内存量
void zfree(void *ptr) {
    void *realptr;
    size_t oldsize;

    if (ptr == NULL) return;
    // 获取存储在内存块开头的实际指针
    realptr = (char*)ptr-sizeof(size_t);
    // 从realptr获取内存块的实际大小
    oldsize = *((size_t*)realptr);
    // 更新已使用内存字节数
    used_memory -= oldsize+sizeof(size_t);
    // 释放实际的内存块
    free(realptr);
}

// 复制字符串
char *zstrdup(const char *s) {
    // 计算字符串长度
    size_t l = strlen(s)+1;
    // 分配内存
    char *p = zmalloc(l);
    // 复制字符串内容
    memcpy(p,s,l);
    // 返回复制后的新字符串指针
    return p;
}

size_t zmalloc_used_memory(void) {
    return used_memory;
}
