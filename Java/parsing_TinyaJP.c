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
#include "parsing_TinyaJP.h"
#include "../ast.h"
#include "../tokenizer.h"
#include "../bootstrap.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


__inline void* jlong2ptr(jlong l) {
	size_t k=(size_t)l;
	return (void*)k;
}

__inline jlong ptr2jlong(void*p) {
	size_t k=(size_t)p;
	return (jlong)k;
}



jobject parse_buffer(JNIEnv* env, jobject parser, const char*buffer,size_t length) {
	token_context_t* toktext;
	ast_node_t*ast,*output;
	jfieldID ast_id,root_id,toktext_id;
	jmethodID cid;
	jclass parserClass,astClass;
	jobject astObj;
	int i=0;
	static char debug[4096],*str=debug;

//	printf("prout %i\n",i+=1);
	parserClass = (*env)->GetObjectClass(env, parser);
//	printf("prout %i\n",i+=1);
	ast_id = (*env)->GetFieldID(env,parserClass,"gram","Lparsing/AST;");
//	printf("prout %i\n",i+=1);

	astObj = (*env)->GetObjectField(env,parser,ast_id);
//	printf("prout %i\n",i+=1);

	astClass = (*env)->GetObjectClass(env,astObj);
//	printf("prout %i\n",i+=1);
	root_id = (*env)->GetFieldID(env,astClass,"res_id","J");
//	printf("prout %i\n",i+=1);

	ast = jlong2ptr( (*env)->GetIntField(env,astObj,root_id) );
//	printf("prout %i\n",i+=1);

	toktext_id = (*env)->GetFieldID(env,parserClass,"tok_context","J");
	toktext = jlong2ptr( (*env)->GetIntField(env,parser,toktext_id) );

	if(toktext) {
		token_context_free(toktext);
	}

	toktext=token_context_new(buffer,length,"[ \t\r\n]*",ast,STRIP_TERMINALS);

	(*env)->SetIntField(env,parser,toktext_id, ptr2jlong(toktext) );

//	printf("prout %i\n",i+=1);
	output=token_produce_any(toktext,find_nterm(ast,"_start"),toktext->flags&STRIP_TERMINALS);
//	printf("prout %i\n",i+=1);
	output=clean_ast(output);
//	printf("prout %i\n",i+=1);
//	if(!output) {
//		printf(parse_error(toktext));
//	}
//	printf("prout %i\n",i+=1);

	cid = (*env)->GetMethodID(env, astClass, "<init>", "(J)V");
//	printf("prout %i\n",i+=1);

	if(isPair(output)) {
		str=debug;
		ast_serialize(output,&str);
		//printf("AST : %s\n",debug);
	} else if(isAtom(output)) {
		printf("oops Atom %s\n",getAtom(output));
	}

	return (*env)->NewObject(env, astClass, cid, ptr2jlong(output));
}



/*
 * Class:     parsing_TinyaJP
 * Method:    parseFile
 * Signature: (Ljava/lang/String;)Lparsing/AST;
 */
JNIEXPORT jobject JNICALL Java_parsing_TinyaJP_parseFile
  (JNIEnv *env, jobject parser, jstring fileName)
{
	struct stat st;
	char*buffer;
	jobject ret;
	FILE*f;

	const char*filename=(*env)->GetStringUTFChars(env, fileName, 0);

	if(stat(filename,&st)) {
		perror("stat");
	}

	f=fopen(filename,"r");
	buffer=(char*) malloc(st.st_size);
	fread(buffer,1,st.st_size,f);
	fclose(f);

	ret=parse_buffer(env,parser,buffer,st.st_size);
	(*env)->ReleaseStringUTFChars(env, fileName, filename);
	free(buffer);
	return ret;
}

/*
 * Class:     parsing_TinyaJP
 * Method:    parseString
 * Signature: (Ljava/lang/String;)Lparsing/AST;
 */
JNIEXPORT jobject JNICALL Java_parsing_TinyaJP_parseString
  (JNIEnv *env, jobject parser, jstring jbuffer) {
	const char*buffer = (*env)->GetStringUTFChars(env, jbuffer, 0);
	jobject ret;

	//printf("got \"%s\"\n",buffer);
	ret=parse_buffer(env,parser,buffer,strlen(buffer));
	//printf("got [%p]\n",ret);
	(*env)->ReleaseStringUTFChars(env, jbuffer, buffer);
	return ret;
}

/*
 * Class:     parsing_TinyaJP
 * Method:    lastError
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_parsing_TinyaJP_lastError
  (JNIEnv *env, jobject parser) {
	token_context_t*
		toktext = jlong2ptr( 
			(*env)->GetIntField(env,parser,
				(*env)->GetFieldID(env,
					(*env)->GetObjectClass(env, parser),
					"tok_context","J")) );
	return (*env)->NewStringUTF(env,
			parse_error(toktext)
		);
}


JNIEXPORT jint JNICALL Java_parsing_TinyaJP_lastErrorLine
  (JNIEnv *env, jobject parser) {
	token_context_t*
		toktext = jlong2ptr( 
			(*env)->GetIntField(env,parser,
				(*env)->GetFieldID(env,
					(*env)->GetObjectClass(env, parser),
					"tok_context","J")) );
	return parse_error_line(toktext);
}


JNIEXPORT jint JNICALL Java_parsing_TinyaJP_lastErrorColumn
  (JNIEnv *env, jobject parser) {
	token_context_t*
		toktext = jlong2ptr( 
			(*env)->GetIntField(env,parser,
				(*env)->GetFieldID(env,
					(*env)->GetObjectClass(env, parser),
					"tok_context","J")) );
	return parse_error_column(toktext);
}

