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
package parsing;

public class TinyaJP {
	private AST gram;
	private long tok_context;

	public TinyaJP() { gram=AST.nil; }
	public TinyaJP(AST ram) { gram	// haha
				       = new AST(ram);
				  // ... gram.
				}

	public AST grammar() { return gram; }

	public native AST parseFile(String fileName);
	public native AST parseString(String str);

	public native String lastError();
	public native int lastErrorLine();
	public native int lastErrorColumn();

	static {
		System.loadLibrary("parsing_TinyaJP");
	}
}

