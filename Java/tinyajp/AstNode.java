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

public class AstNode {
	static final public AstNode nil=new AstNode();

	/** resource id ; pointeur utilisé par la lib native */
	private long ptr;

	private AstNode() { ptr=0; }

	public AstNode(long ptr) { this.ptr = ptr; }

	public boolean	isNil() { return Native.nodeIsNil(ptr); }

	public boolean	isString() { return Native.nodeIsString(ptr); }
	public String	getString() { return Native.nodeGetString(ptr); }

	public boolean	isList() { return Native.nodeIsList(ptr); }
	public int	getSize() { return Native.nodeGetOperandCount(ptr)+1; }
	public AstNode	getElementAt(int i) { return new AstNode(Native.listGetElement(ptr,i)); }

	public int	getRow() { return Native.nodeGetRow(ptr); }
	public int	getCol() { return Native.nodeGetCol(ptr); }


	public boolean	isOp() { return Native.nodeIsList(ptr) && Native.nodeIsString(Native.listGetElement(ptr,0)); }
	public String	getOperator() { return Native.nodeGetOperator(ptr); }
	public int	getOperandCount() { return Native.nodeGetOperandCount(ptr); }
	public AstNode	getOperand(int i) { return new AstNode(Native.nodeGetOperand(ptr,i)); }

	public String toString() { return Native.nodeToString(ptr); }
	public void toFile(String fnam) { Native.nodeToFile(ptr,fnam); }

	long getHandle() { return ptr; }
}

