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


#include <jni.h>
/* Header for class tinyajp_Native */

#include "../tinyap.h"

#ifndef _Included_tinyajp_Native
#define _Included_tinyajp_Native
#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

	__inline void* jlong2ptr(jlong l) {
	        size_t k=(size_t)l;
        	return (void*)k;
	}

	__inline jlong ptr2jlong(void*p) {
        	size_t k=(size_t)p;
	        return (jlong)k;
	}


/*
 * Class:     tinyajp_Native
 * Method:    parserNew
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_tinyajp_Native_parserNew
  (JNIEnv *env, jclass _native_)
{
	return ptr2jlong( tinyap_new() );
}


/*
 * Class:     tinyajp_Native
 * Method:    parserGetWhitespace
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_tinyajp_Native_parserGetWhitespace
  (JNIEnv *env, jclass _native_, jlong parser)
{
	return (*env)->NewStringUTF(env,
			tinyap_get_whitespace(jlong2ptr(parser)));
}


/*
 * Class:     tinyajp_Native
 * Method:    parserSetWhitespace
 * Signature: (JLjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_tinyajp_Native_parserSetWhitespace
  (JNIEnv *env, jclass _native_, jlong parser, jstring ws)
{
	const char *ws_cstr = (*env)->GetStringUTFChars(env, ws, 0);
	tinyap_set_whitespace(jlong2ptr(parser),ws_cstr);
	(*env)->ReleaseStringUTFChars(env, ws, ws_cstr);
}


/*
 * Class:     tinyajp_Native
 * Method:    parserSetWhitespaceRegexp
 * Signature: (JLjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_tinyajp_Native_parserSetWhitespaceRegexp
  (JNIEnv *env, jclass _native_, jlong parser, jstring wsre)
{
	const char *wsre_cstr = (*env)->GetStringUTFChars(env, wsre, 0);
	tinyap_set_whitespace_regexp(jlong2ptr(parser),wsre_cstr);
	(*env)->ReleaseStringUTFChars(env, wsre, wsre_cstr);
}


/*
 * Class:     tinyajp_Native
 * Method:    parserGetGrammar
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_tinyajp_Native_parserGetGrammar
  (JNIEnv *env, jclass _native_, jlong parser)
{
	return (*env)->NewStringUTF(env,
			tinyap_get_grammar(jlong2ptr(parser)));
}


/*
 * Class:     tinyajp_Native
 * Method:    parserSetGrammar
 * Signature: (JLjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_tinyajp_Native_parserSetGrammar
  (JNIEnv *env, jclass _native_, jlong parser, jstring gram)
{
	const char *gram_cstr = (*env)->GetStringUTFChars(env, gram, 0);
	tinyap_set_grammar(jlong2ptr(parser),gram_cstr);
	(*env)->ReleaseStringUTFChars(env, gram, gram_cstr);
}


/*
 * Class:     tinyajp_Native
 * Method:    parserGetGrammarAst
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_tinyajp_Native_parserGetGrammarAst
  (JNIEnv *env, jclass _native_, jlong parser)
{
	return ptr2jlong( tinyap_get_grammar_ast(jlong2ptr(parser)) );
}


/*
 * Class:     tinyajp_Native
 * Method:    parserSetGrammarAst
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_tinyajp_Native_parserSetGrammarAst
  (JNIEnv *env, jclass _native_, jlong parser, jlong gast)
{
	tinyap_set_grammar(jlong2ptr(parser),jlong2ptr(gast));
}


/*
 * Class:     tinyajp_Native
 * Method:    parserGetSourceFile
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_tinyajp_Native_parserGetSourceFile
  (JNIEnv *env, jclass _native_, jlong parser)
{
	return (*env)->NewStringUTF(env,
			tinyap_get_source_file(jlong2ptr(parser)));
}


/*
 * Class:     tinyajp_Native
 * Method:    parserGetSourceBuffer
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_tinyajp_Native_parserGetSourceBuffer
  (JNIEnv *env, jclass _native_, jlong parser)
{
	return (*env)->NewStringUTF(env,
			tinyap_get_source_buffer(jlong2ptr(parser)));
}


/*
 * Class:     tinyajp_Native
 * Method:    parserSetSourceFile
 * Signature: (JLjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_tinyajp_Native_parserSetSourceFile
  (JNIEnv *env, jclass _native_, jlong parser, jstring fnam)
{
	const char *fnam_cstr = (*env)->GetStringUTFChars(env, fnam, 0);
	tinyap_set_source_file(jlong2ptr(parser),fnam_cstr);
	(*env)->ReleaseStringUTFChars(env, fnam, fnam_cstr);
}


/*
 * Class:     tinyajp_Native
 * Method:    parserSetSourceBuffer
 * Signature: (JLjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_tinyajp_Native_parserSetSourceBuffer
  (JNIEnv *env, jclass _native_, jlong parser, jstring buffer)
{
	const char *buffer_cstr = (*env)->GetStringUTFChars(env, buffer, 0);
	int buffer_sz = (*env)->GetStringUTFLength(env,buffer);
	fprintf(stderr,"got buffer [%i] <<<EOD\n%s\nEOD;\n%i\n",buffer_sz,buffer_cstr,(int)*(buffer_cstr+buffer_sz));
	tinyap_set_source_buffer(jlong2ptr(parser),buffer_cstr,buffer_sz);
	(*env)->ReleaseStringUTFChars(env, buffer, buffer_cstr);
}


/*
 * Class:     tinyajp_Native
 * Method:    parserParse
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_tinyajp_Native_parserParse
  (JNIEnv *env, jclass _native_, jlong parser)
{
	return (jboolean)tinyap_parse(jlong2ptr(parser));
}


/*
 * Class:     tinyajp_Native
 * Method:    parserParseAsGrammar
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_tinyajp_Native_parserParseAsGrammar
  (JNIEnv *env, jclass _native_, jlong parser)
{
	return (jboolean)tinyap_parse_as_grammar(jlong2ptr(parser));
}


/*
 * Class:     tinyajp_Native
 * Method:    parserParsedOK
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_tinyajp_Native_parserParsedOK
  (JNIEnv *env, jclass _native_, jlong parser)
{
	return (jboolean)tinyap_parsed_ok(jlong2ptr(parser));
}


/*
 * Class:     tinyajp_Native
 * Method:    parserGetOutput
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_tinyajp_Native_parserGetOutput
  (JNIEnv *env, jclass _native_, jlong parser)
{
	return ptr2jlong(tinyap_get_output(jlong2ptr(parser)));
}


/*
 * Class:     tinyajp_Native
 * Method:    parserGetErrorCol
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_tinyajp_Native_parserGetErrorCol
  (JNIEnv *env, jclass _native_, jlong parser)
{
	return (jint)tinyap_get_error_col(jlong2ptr(parser));
}


/*
 * Class:     tinyajp_Native
 * Method:    parserGetErrorRow
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_tinyajp_Native_parserGetErrorRow
  (JNIEnv *env, jclass _native_, jlong parser)
{
	return (jint)tinyap_get_error_row(jlong2ptr(parser));
}


/*
 * Class:     tinyajp_Native
 * Method:    parserGetError
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_tinyajp_Native_parserGetError
  (JNIEnv *env, jclass _native_, jlong parser)
{
	return (*env)->NewStringUTF(env,
			tinyap_get_error(jlong2ptr(parser)));
}


/*
 * Class:     tinyajp_Native
 * Method:    parserDelete
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_tinyajp_Native_parserDelete
  (JNIEnv *env, jclass _native_, jlong parser)
{
	tinyap_delete(jlong2ptr(parser));
}


/*
 * Class:     tinyajp_Native
 * Method:    nodeIsNil
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_tinyajp_Native_nodeIsNil
  (JNIEnv *env, jclass _native_, jlong node)
{
	return (jboolean)tinyap_node_is_nil(jlong2ptr(node));
}


/*
 * Class:     tinyajp_Native
 * Method:    nodeIsList
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_tinyajp_Native_nodeIsList
  (JNIEnv *env, jclass _native_, jlong node)
{
	return (jboolean)tinyap_node_is_list(jlong2ptr(node));
}


/*
 * Class:     tinyajp_Native
 * Method:    listGetElement
 * Signature: (JI)J
 */
JNIEXPORT jlong JNICALL Java_tinyajp_Native_listGetElement
  (JNIEnv *env, jclass _native_, jlong node, jint i)
{
	return ptr2jlong(tinyap_list_get_element(jlong2ptr(node),i));
}


/*
 * Class:     tinyajp_Native
 * Method:    nodeIsString
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_tinyajp_Native_nodeIsString
  (JNIEnv *env, jclass _native_, jlong node)
{
	return (jboolean)tinyap_node_is_string(jlong2ptr(node));
}


/*
 * Class:     tinyajp_Native
 * Method:    nodeGetString
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_tinyajp_Native_nodeGetString
  (JNIEnv *env, jclass _native_, jlong node)
{
	return (*env)->NewStringUTF(env,
			tinyap_node_get_string(jlong2ptr(node)));
}


/*
 * Class:     tinyajp_Native
 * Method:    nodeGetOperator
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_tinyajp_Native_nodeGetOperator
  (JNIEnv *env, jclass _native_, jlong node)
{
	return (*env)->NewStringUTF(env,
			tinyap_node_get_operator(jlong2ptr(node)));
}


/*
 * Class:     tinyajp_Native
 * Method:    nodeGetOperand
 * Signature: (JI)J
 */
JNIEXPORT jlong JNICALL Java_tinyajp_Native_nodeGetOperand
  (JNIEnv *env, jclass _native_, jlong node, jint i)
{
	return ptr2jlong(tinyap_node_get_operand(jlong2ptr(node),i));
				
}


/*
 * Class:     tinyajp_Native
 * Method:    nodeGetOperandCount
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_tinyajp_Native_nodeGetOperandCount
  (JNIEnv *env, jclass _native_, jlong node)
{
	return (jint)tinyap_node_get_operand_count(jlong2ptr(node));
}


/*
 * Class:     tinyajp_Native
 * Method:    nodeToString
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_tinyajp_Native_nodeToString
  (JNIEnv *env, jclass _native_, jlong node)
{
	return (*env)->NewStringUTF(env,
			tinyap_serialize_to_string(jlong2ptr(node)));
}


/*
 * Class:     tinyajp_Native
 * Method:    nodeToFile
 * Signature: (JLjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_tinyajp_Native_nodeToFile
  (JNIEnv *env, jclass _native_, jlong node, jstring fnam)
{
	const char *fnam_cstr = (*env)->GetStringUTFChars(env, fnam, 0);
	tinyap_serialize_to_file(jlong2ptr(node),fnam_cstr);
	(*env)->ReleaseStringUTFChars(env, fnam, fnam_cstr);
}


/*
 * Class:     tinyajp_Native
 * Method:    nodeGetRow
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_tinyajp_Native_nodeGetRow
  (JNIEnv *env, jclass _native_, jlong node)
{
	return (jint)tinyap_node_get_row(jlong2ptr(node));
}


/*
 * Class:     tinyajp_Native
 * Method:    nodeGetCol
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_tinyajp_Native_nodeGetCol
  (JNIEnv *env, jclass _native_, jlong node)
{
	return (jint)tinyap_node_get_col(jlong2ptr(node));
}


#ifdef __cplusplus
}
#endif
#endif
