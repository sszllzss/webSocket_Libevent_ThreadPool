/*************************************************************************
# > File Name: debug.h
# > Author: SSZL
# > Mail: sszllzss@foxmail.com
# > Blog: sszlbg.cn
# > Created Time: 2018-09-16 15:30:40
# > Revise Time: 2018-09-16 15:55:33
 ************************************************************************/

#ifndef _DEBUG_H
#define _DEBUG_H
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<error.h>
#include<errno.h>
#include<err.h>
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
#endif
