/* Tinya(J)P : this is not yet another (Java) parser.
 * Copyright (C) 2007 Damien Leroux
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include "parsing_AST.h"
#include "../ast.h"
#include "../tokenizer.h"
#include "../bootstrap.h"

#ifdef __cplusplus
extern "C" {
#endif

__inline void* jlong2ptr(jlong l) {
	size_t k=(size_t)l;
	return (void*)k;
}

__inline jlong ptr2jlong(void*p) {
	size_t k=(size_t)p;
	return (jlong)k;
}

/*
 * Class:     parsing_AST
 * Method:    isPair
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_parsing_AST_isPair
  (JNIEnv *env, jclass ast, jlong ptr)
{
	return (jboolean)isPair( (ast_node_t*) jlong2ptr(ptr) );
}

/*
 * Class:     parsing_AST
 * Method:    isAtom
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_parsing_AST_isAtom
  (JNIEnv *env, jclass ast, jlong ptr)
{
	return (jboolean)isAtom( (ast_node_t*) jlong2ptr(ptr) );
}

/*
 * Class:     parsing_AST
 * Method:    size
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_parsing_AST_size
  (JNIEnv *env, jclass ast, jlong ptr)
{
	ast_node_t* a=(ast_node_t*)jlong2ptr(ptr);
	jint ret=0;

	while(a) {
		ret+=1;
		a=getCdr(a);
	}

	return ret;
}

/*
 * Class:     parsing_AST
 * Method:    getAtom
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_parsing_AST_getAtom
  (JNIEnv *env, jclass ast, jlong atom)
{
	return (*env)->NewStringUTF(env, getAtom((ast_node_t*)jlong2ptr(atom)));
}

/*
 * Class:     parsing_AST
 * Method:    getCar
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_parsing_AST_getCar
  (JNIEnv *env, jclass ast, jlong pair)
{
	return ptr2jlong(getCar((ast_node_t*)jlong2ptr(pair)));
}

/*
 * Class:     parsing_AST
 * Method:    getCdr
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_parsing_AST_getCdr
  (JNIEnv *env, jclass ast, jlong pair)
{
	return ptr2jlong(getCdr((ast_node_t*)jlong2ptr(pair)));
}


JNIEXPORT jint JNICALL Java_parsing_AST_getRow
  (JNIEnv *env, jclass ast, jlong pair)
{
	return (jint)getRow((ast_node_t*)jlong2ptr(pair));
}


JNIEXPORT jint JNICALL Java_parsing_AST_getCol
  (JNIEnv *env, jclass ast, jlong pair)
{
	return (jint)getCol((ast_node_t*)jlong2ptr(pair));
}




/*
 * Class:     parsing_AST
 * Method:    get
 * Signature: (Ljava/lang/String;)Lparsing/AST;
 */
JNIEXPORT jobject JNICALL Java_parsing_AST_get
  (JNIEnv *env, jclass ast, jstring which)
{
	jfieldID fid;
	jmethodID cid;
	jobject ret=0;
	static char debug[4096],*str=debug;
	ast_node_t*ptr=NULL;
	const char *astName = (*env)->GetStringUTFChars(env, which, 0);

	//fprintf(stderr,"On demande l'AST '%s'...\n",astName);
	ptr=get_ruleset(astName);

	if(ptr) {
		//fprintf(stderr,"On a trouvé un AST [%p]\n",ptr);

		//dump_node(ptr);
		//memset(debug,0,4096);
		//ast_serialize(ptr,&str);
		//printf("AST : %s\n",debug);

		cid = (*env)->GetMethodID(env, ast, "<init>", "(J)V");
//		fprintf(stderr,"L'id du constructeur est [%p]\n",cid);
		ret=(*env)->NewObject(env, ast, cid, ptr2jlong(ptr));
//		fprintf(stderr,"On a maintenant un jobject à [%p]\n",ret);
	} else {
		fprintf(stderr,"AST %s not found.\n",astName);
	}

	(*env)->ReleaseStringUTFChars(env, which, astName);

	return ret;
}

#ifdef __cplusplus
}
#endif

