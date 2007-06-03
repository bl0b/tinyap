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

public class AST {
	static final public AST nil=new AST();


	private	Node root;

	/** resource id ; pointeur utilisé par la lib native */
	private long res_id;

	/** node permet de lire un noeud */
	public class Node {
		public Node(long ptr) {
			this.ptr = ptr;
			initCache(ptr,0);
		}

		public boolean isString() { return AST.isAtom(ptr); }
		public boolean isList() { return AST.isPair(ptr); }
		public boolean isNil() { return AST.isNil(ptr); }
		public int size() { if(isList()) return cache.length; return 1; }
		public String getString() { return AST.getAtom(ptr); }

		public int getRow() { return AST.getRow(ptr); }
		public int getCol() { return AST.getCol(ptr); }

		public Node getNodeAt(int i) { if(isList()) { return cache[i]; } return null; }

		private void initCache(long ptr,int length) {
			//System.out.println("["+ptr+"]"+"  "+length);
			if(AST.isAtom(ptr)) {
				//System.out.println("=> Atom "+AST.getAtom(ptr));
				if(length==0) {
					cache=null;
				} else {
					cache=new Node[length+1];
					cache[length]=new Node(ptr);
				}
			} else if(AST.isPair(ptr)) {
				/* termine d'abord le calcul de la longueur et l'initialisation du tableau */
				initCache(AST.getCdr(ptr),length+1);
				/* ajouter l'item courant au tableau */
				cache[length]=new Node(AST.getCar(ptr));
			} else if(AST.isNil(ptr)) {
				cache = new Node[length];
			}
		}

		private long ptr;
		private Node[] cache;
	}

	public AST() { res_id=0; root = null; }
	       AST(long ptr) { res_id=ptr; root = new Node(res_id); }
	public AST(String astName) { if(get(astName)!=null) { res_id=get(astName).res_id; root = new Node(res_id); } }
	public AST(AST cp) { if(cp==null) { res_id=0; } else { res_id=cp.res_id; } root = new Node(res_id); }

	public Node getRoot() { return root; }

	private static native boolean isPair(long ptr);
	private static native boolean isAtom(long ptr);
	private static        boolean isNil(long ptr) { return ptr==0; }
	private static native int size(long ptr);
	private static native String getAtom(long ptr);
	private static native long getCar(long ptr);
	private static native long getCdr(long ptr);
	private static native int getRow(long ptr);
	private static native int getCol(long ptr);

	public long debug_id() { return res_id; }

	/** récupère un AST prédéfini (par exemple BNF_ext_1) */
	native public static AST get(String astName);

	static {
		System.loadLibrary("parsing_AST");
	}
}
