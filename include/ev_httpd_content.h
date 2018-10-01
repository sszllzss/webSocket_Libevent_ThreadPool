/*************************************************************************
# > File Name: ev_httpd_content.h
# > Author: SSZL
# > Mail: sszllzss@foxmail.com
# > Blog: sszlbg.cn
# > Created Time: 2018-09-24 14:15:33
# > Revise Time: 2018-10-01 15:18:49
 ************************************************************************/

#ifndef _EV_HTTPD_CONTENT_H
#define _EV_HTTPD_CONTENT_H
 /* 错误码还回数据  */
#define HTTP_NOCONTENT_CONTENT \
    "<html>\r\n"\
    "	<head>\r\n"\
    "		<meta charset=\"utf-8\">\r\n"\
    "		<title>204  请求没有内容 </title>\r\n"\
    "	</head>\r\n"\
    "	<body >\r\n"\
    "	<div  style=\"text-align:center\">\r\n"\
    "			<center>\r\n"\
    "			<div >\r\n"\
    "				<p >\r\n"\
    "					<h1 style=\"text-align:center;padding-top:20px; padding-bottom:20px;\">\r\n"\
    "						 204 请求没有内容\r\n"\
    "					</h1>\r\n"\
    "				</p>\r\n"\
    "			</div>\r\n"\
    "			<div  style=\"text-align:center;margin-top:10px\">\r\n"\
    "			   <div>  版权所有 ©<a href=\"https://sszlbg.cn\">SSZL博客</a></div>\r\n"\
    "			</div>\r\n"\
    "			</center>\r\n"\
    "	</div>\r\n"\
    "	</body>\r\n"\
    "</html>"
#define HTTP_MOVEPERM_CONTENT \
    "<html>\r\n"\
    "	<head>\r\n"\
    "		<meta charset=\"utf-8\">\r\n"\
    "		<title>301  资源（网页等）被永久转移到其它URL </title>\r\n"\
    "	</head>\r\n"\
    "	<body >\r\n"\
    "	<div  style=\"text-align:center\">\r\n"\
    "			<center>\r\n"\
    "			<div >\r\n"\
    "				<p >\r\n"\
    "					<h1 style=\"text-align:center;padding-top:20px; padding-bottom:20px;\">\r\n"\
    "						 301 资源（网页等）被永久转移到其它URL\r\n"\
    "					</h1>\r\n"\
    "				</p>\r\n"\
    "			</div>\r\n"\
    "			<div  style=\"text-align:center;margin-top:10px\">\r\n"\
    "			   <div>  版权所有 ©<a href=\"https://sszlbg.cn\">SSZL博客</a></div>\r\n"\
    "			</div>\r\n"\
    "			</center>\r\n"\
    "	</div>\r\n"\
    "	</body>\r\n"\
    "</html>"
#define HTTP_MOVETEMP_CONTENT \
    "<html>\r\n"\
    "	<head>\r\n"\
    "		<meta charset=\"utf-8\">\r\n"\
    "		<title>302  重定向或网址劫持 </title>\r\n"\
    "	</head>\r\n"\
    "	<body >\r\n"\
    "	<div  style=\"text-align:center\">\r\n"\
    "			<center>\r\n"\
    "			<div >\r\n"\
    "				<p >\r\n"\
    "					<h1 style=\"text-align:center;padding-top:20px; padding-bottom:20px;\">\r\n"\
    "						 302 重定向或网址劫持\r\n"\
    "					</h1>\r\n"\
    "				</p>\r\n"\
    "			</div>\r\n"\
    "			<div  style=\"text-align:center;margin-top:10px\">\r\n"\
    "			   <div>  版权所有 ©<a href=\"https://sszlbg.cn\">SSZL博客</a></div>\r\n"\
    "			</div>\r\n"\
    "			</center>\r\n"\
    "	</div>\r\n"\
    "	</body>\r\n"\
    "</html>"
#define HTTP_BADREQUEST_CONTENT \
    "<html>\r\n"\
    "	<head>\r\n"\
    "		<meta charset=\"utf-8\">\r\n"\
    "		<title>400  无效的http请求 </title>\r\n"\
    "	</head>\r\n"\
    "	<body >\r\n"\
    "	<div  style=\"text-align:center\">\r\n"\
    "			<center>\r\n"\
    "			<div >\r\n"\
    "				<p >\r\n"\
    "					<h1 style=\"text-align:center;padding-top:20px; padding-bottom:20px;\">\r\n"\
    "						 400 无效的http请求\r\n"\
    "					</h1>\r\n"\
    "				</p>\r\n"\
    "			</div>\r\n"\
    "			<div  style=\"text-align:center;margin-top:10px\">\r\n"\
    "			   <div>  版权所有 ©<a href=\"https://sszlbg.cn\">SSZL博客</a></div>\r\n"\
    "			</div>\r\n"\
    "			</center>\r\n"\
    "	</div>\r\n"\
    "	</body>\r\n"\
    "</html>"
#define HTTP_NOTFOUND_CONTENT \
    "<html>\r\n"\
    "	<head>\r\n"\
    "		<meta charset=\"utf-8\">\r\n"\
    "		<title>404 找不到uri的内容  </title>\r\n"\
    "	</head>\r\n"\
    "	<body >\r\n"\
    "	<div  style=\"text-align:center\">\r\n"\
    "			<center>\r\n"\
    "			<div >\r\n"\
    "				<p >\r\n"\
    "					<h1 style=\"text-align:center;padding-top:20px; padding-bottom:20px;\">\r\n"\
    "						 404 资源不存在！！！\r\n"\
    "					</h1>\r\n"\
    "				</p>\r\n"\
    "			</div>\r\n"\
    "			<div  style=\"text-align:center;margin-top:10px\">\r\n"\
    "			   <div>  版权所有 ©<a href=\"https://sszlbg.cn\">SSZL博客</a></div>\r\n"\
    "			</div>\r\n"\
    "			</center>\r\n"\
    "	</div>\r\n"\
    "	</body>\r\n"\
    "</html>"
#define HTTP_BADMETHOD_CONTENT \
    "<html>\r\n"\
    "	<head>\r\n"\
    "		<meta charset=\"utf-8\">\r\n"\
    "		<title>405 方法禁用  </title>\r\n"\
    "	</head>\r\n"\
    "	<body >\r\n"\
    "	<div  style=\"text-align:center\">\r\n"\
    "			<center>\r\n"\
    "			<div >\r\n"\
    "				<p >\r\n"\
    "					<h1 style=\"text-align:center;padding-top:20px; padding-bottom:20px;\">\r\n"\
    "						 405 方法禁用\r\n"\
    "					</h1>\r\n"\
    "				</p>\r\n"\
    "			</div>\r\n"\
    "			<div  style=\"text-align:center;margin-top:10px\">\r\n"\
    "			   <div>  版权所有 ©<a href=\"https://sszlbg.cn\">SSZL博客</a></div>\r\n"\
    "			</div>\r\n"\
    "			</center>\r\n"\
    "	</div>\r\n"\
    "	</body>\r\n"\
    "</html>"
#define HTTP_ENTITYTOOLARGE_CONTENT \
    "<html>\r\n"\
    "	<head>\r\n"\
    "		<meta charset=\"utf-8\">\r\n"\
    "		<title>413 请求实体太大  </title>\r\n"\
    "	</head>\r\n"\
    "	<body >\r\n"\
    "	<div  style=\"text-align:center\">\r\n"\
    "			<center>\r\n"\
    "			<div >\r\n"\
    "				<p >\r\n"\
    "					<h1 style=\"text-align:center;padding-top:20px; padding-bottom:20px;\">\r\n"\
    "						 413 请求实体太大\r\n"\
    "					</h1>\r\n"\
    "				</p>\r\n"\
    "			</div>\r\n"\
    "			<div  style=\"text-align:center;margin-top:10px\">\r\n"\
    "			   <div>  版权所有 ©<a href=\"https://sszlbg.cn\">SSZL博客</a></div>\r\n"\
    "			</div>\r\n"\
    "			</center>\r\n"\
    "	</div>\r\n"\
    "	</body>\r\n"\
    "</html>"
#define HTTP_EXPECTATIONFAILED_CONTENT \
    "<html>\r\n"\
    "	<head>\r\n"\
    "		<meta charset=\"utf-8\">\r\n"\
    "		<title>417 预期结果失败  </title>\r\n"\
    "	</head>\r\n"\
    "	<body >\r\n"\
    "	<div  style=\"text-align:center\">\r\n"\
    "			<center>\r\n"\
    "			<div >\r\n"\
    "				<p >\r\n"\
    "					<h1 style=\"text-align:center;padding-top:20px; padding-bottom:20px;\">\r\n"\
    "						 417 预期结果失败\r\n"\
    "					</h1>\r\n"\
    "				</p>\r\n"\
    "			</div>\r\n"\
    "			<div  style=\"text-align:center;margin-top:10px\">\r\n"\
    "			   <div>  版权所有 ©<a href=\"https://sszlbg.cn\">SSZL博客</a></div>\r\n"\
    "			</div>\r\n"\
    "			</center>\r\n"\
    "	</div>\r\n"\
    "	</body>\r\n"\
    "</html>"
#define HTTP_INTERNAL_CONTENT \
    "<html>\r\n"\
    "	<head>\r\n"\
    "		<meta charset=\"utf-8\">\r\n"\
    "		<title>500 服务器内部错误  </title>\r\n"\
    "	</head>\r\n"\
    "	<body >\r\n"\
    "	<div  style=\"text-align:center\">\r\n"\
    "			<center>\r\n"\
    "			<div >\r\n"\
    "				<p >\r\n"\
    "					<h1 style=\"text-align:center;padding-top:20px; padding-bottom:20px;\">\r\n"\
    "						 500 服务器内部错误\r\n"\
    "					</h1>\r\n"\
    "				</p>\r\n"\
    "			</div>\r\n"\
    "			<div  style=\"text-align:center;margin-top:10px\">\r\n"\
    "			   <div>  版权所有 ©<a href=\"https://sszlbg.cn\">SSZL博客</a></div>\r\n"\
    "			</div>\r\n"\
    "			</center>\r\n"\
    "	</div>\r\n"\
    "	</body>\r\n"\
    "</html>"
#define HTTP_NOTIMPLEMENTED_CONTENT \
    "<html>\r\n"\
    "	<head>\r\n"\
    "		<meta charset=\"utf-8\">\r\n"\
    "		<title>501 服务器不具备完成请求的功能  </title>\r\n"\
    "	</head>\r\n"\
    "	<body >\r\n"\
    "	<div  style=\"text-align:center\">\r\n"\
    "			<center>\r\n"\
    "			<div >\r\n"\
    "				<p >\r\n"\
    "					<h1 style=\"text-align:center;padding-top:20px; padding-bottom:20px;\">\r\n"\
    "						 501 服务器不具备完成请求的功能 <br/>例如,服务器无法识别请求方法时可能会返回此代码\r\n"\
    "					</h1>\r\n"\
    "				</p>\r\n"\
    "			</div>\r\n"\
    "			<div  style=\"text-align:center;margin-top:10px\">\r\n"\
    "			   <div>  版权所有 ©<a href=\"https://sszlbg.cn\">SSZL博客</a></div>\r\n"\
    "			</div>\r\n"\
    "			</center>\r\n"\
    "	</div>\r\n"\
    "	</body>\r\n"\
    "</html>"
#define HTTP_SERVUNAVAIL_CONTENT \
    "<html>\r\n"\
    "	<head>\r\n"\
    "		<meta charset=\"utf-8\">\r\n"\
    "		<title>503 服务器不可用  </title>\r\n"\
    "	</head>\r\n"\
    "	<body >\r\n"\
    "	<div  style=\"text-align:center\">\r\n"\
    "			<center>\r\n"\
    "			<div >\r\n"\
    "				<p >\r\n"\
    "					<h1 style=\"text-align:center;padding-top:20px; padding-bottom:20px;\">\r\n"\
    "						 503 服务器正在维护。 \r\n"\
    "					</h1>\r\n"\
    "				</p>\r\n"\
    "			</div>\r\n"\
    "			<div  style=\"text-align:center;margin-top:10px\">\r\n"\
    "			   <div>  版权所有 ©<a href=\"https://sszlbg.cn\">SSZL博客</a></div>\r\n"\
    "			</div>\r\n"\
    "			</center>\r\n"\
    "	</div>\r\n"\
    "	</body>\r\n"\
    "</html>"

#endif
