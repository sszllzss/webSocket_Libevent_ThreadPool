/*************************************************************************
# > File Name: include/debug.h
# > Author: SSZL
# > Mail: sszllzss@foxmail.com
# > Blog: sszlbg.cn
# > Created Time: 2018-09-16 15:30:40
# > Revise Time: 2018-10-11 15:13:49
 ************************************************************************/

#ifndef _DEBUG_H
#define _DEBUG_H
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<error.h>
#include<errno.h>
#include<err.h>
#include"config.h"
#ifdef DEBUG_PRINTF
#define De_printf(str,args...) do{\
                                    printf("%s:%s:%d] ", __FUNCTION__, __FILE__, __LINE__);\
                                    printf(str,##args);\
                                    printf("\r\n");\
                                    }while(0) 
#define De_perror(str) do{\
                                    fprintf(stderr,"%s:%s:%d] ", __FUNCTION__, __FILE__, __LINE__);\
                                    perror(str);\
                                    }while(0) 
#define De_fprintf(f,str,args...) do{\
                                    fprintf(f, "%s:%s:%d] ", __FUNCTION__, __FILE__, __LINE__);\
                                    fprintf(f, str, ##args);\
                                    fprintf(f, "\r\n");\
                                    }while(0) 
#else
#define De_printf(str,args...) 
#define De_perror(str)
#define De_fprintf(f,str,args...)
#endif
#endif
