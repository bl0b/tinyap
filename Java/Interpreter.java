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
import parsing.AST;
import parsing.TinyaJP;

public class Interpreter {
	static String readString() {
		String ret=new String();
		char c;

		try {

		c=(char)System.in.read();
		while(c!='\n') {
			ret+=c;
			c=(char)System.in.read();
		}

		} catch(java.io.IOException e) {}

		return ret;
	}


	static double test_math_ast(AST.Node n) {
		if(n.getNodeAt(0).getString().equals("number")) {
			return Double.valueOf(n.getNodeAt(1).getString());
		} else if(n.getNodeAt(0).getString().equals("m_minus")) {
			return - test_math_ast(n.getNodeAt(1));
		} else if(n.getNodeAt(0).getString().equals("m_expr")) {
			return test_math_ast(n.getNodeAt(1));
		} else if(n.getNodeAt(0).getString().equals("m_add")) {
			return test_math_ast(n.getNodeAt(1))
				+ test_math_ast(n.getNodeAt(2));
		} else if(n.getNodeAt(0).getString().equals("m_sub")) {
			return test_math_ast(n.getNodeAt(1))
				- test_math_ast(n.getNodeAt(2));
		} else if(n.getNodeAt(0).getString().equals("m_mul")) {
			return test_math_ast(n.getNodeAt(1))
				* test_math_ast(n.getNodeAt(2));
		} else if(n.getNodeAt(0).getString().equals("m_div")) {
			return test_math_ast(n.getNodeAt(1))
				/ test_math_ast(n.getNodeAt(2));
		} else {
			System.out.println("opérateur inconnu "+n.getNodeAt(0).getString());
			return 0;
		}
	}

	static void dump_node_rec(AST.Node n, String ofs) {
		if(n==null) {
			System.out.println(ofs+"#nil");
		} else if(n.isString()) {
			System.out.println(ofs+n.getString());
		} else {
			System.out.println(ofs+"  [");
			/* n.isList()==true */
			for(int i=0;i<n.size();i++) {
				dump_node_rec(n.getNodeAt(i),ofs+"    ");
			}
			System.out.println(ofs+"  ]");
		}
	}

	/* proxy pour dump_node_rec */
	static void dump_nodes(AST.Node n) {
		dump_node_rec(n,"");
	}

/*	static String readString() {
		String str=new String();
		int ret;
		try {
			ret=System.in.read();
			while(ret!=-1&&ret!='\n') {
				str+=(char)ret;
				ret=System.in.read();
			}
		} catch(java.io.IOException ioe) {
		}
		return str;
	}
*/
}
