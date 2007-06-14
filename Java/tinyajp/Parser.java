/* Tinya(J)P : this is not yet another (Java) parser. - Java binding
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
package tinyajp;

public class Parser {
	private long ptr;

	public Parser() { ptr=Native.parserNew(); }
	public void finalize() { Native.parserDelete(ptr); }

	public String	getWhitespace() { return Native.parserGetWhitespace(ptr); }
	public Parser	setWhitespace(String ws) { Native.parserSetWhitespace(ptr,ws); return this; }
	public Parser	setWhitespaceRegexp(String wsre) { Native.parserSetWhitespaceRegexp(ptr,wsre); return this; }

	public String	getGrammar() { return Native.parserGetGrammar(ptr); }
	public Parser	setGrammar(String g) { Native.parserSetGrammar(ptr,g); return this; }

	public AstNode	getGrammarAst() { return new AstNode(Native.parserGetGrammarAst(ptr)); }
	public Parser	setGrammarAst(AstNode gast) { Native.parserSetGrammarAst(ptr,gast.getHandle()); return this; }

	public String	getSourceFile() { return Native.parserGetSourceFile(ptr); }
	public String	getSourceBuffer() { return Native.parserGetSourceBuffer(ptr); }
	public Parser	setSourceFile(String fnam) { Native.parserSetSourceFile(ptr,fnam); return this; }
	public Parser	setSourceBuffer(String buffer) { Native.parserSetSourceBuffer(ptr,buffer); return this; }

	public boolean	parse() { return Native.parserParse(ptr); }
	public boolean	parseAsGrammar() { return Native.parserParseAsGrammar(ptr); }
	public boolean	parsedOK() { return Native.parserParsedOK(ptr); }

	public AstNode	getOutput() { return new AstNode(Native.parserGetOutput(ptr)); }

	public int	getErrorCol() { return Native.parserGetErrorCol(ptr); }
	public int	getErrorRow() { return Native.parserGetErrorRow(ptr); }
	public String	getError() { return Native.parserGetError(ptr); }

	long getHandle() { return ptr; }
}

