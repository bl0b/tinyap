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
import tinyajp.AstNode;
import tinyajp.Parser;

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


	static double test_math_ast(AstNode n) {
		if(n==null||(!n.isOp())) {
			return 0;
		} else if(n.getOperator().equals("Number")) {
			return Double.valueOf(n.getOperand(0).getString());
		} else if(n.getOperator().equals("MathMinus")) {
			return - test_math_ast(n.getOperand(0));
		} else if(n.getOperator().equals("MathExpr")) {
			return test_math_ast(n.getOperand(0));
		} else if(n.getOperator().equals("MathAdd")) {
			return test_math_ast(n.getOperand(0))
				+ test_math_ast(n.getOperand(1));
		} else if(n.getOperator().equals("MathSub")) {
			return test_math_ast(n.getOperand(0))
				- test_math_ast(n.getOperand(1));
		} else if(n.getOperator().equals("MathMul")) {
			return test_math_ast(n.getOperand(0))
				* test_math_ast(n.getOperand(1));
		} else if(n.getOperator().equals("MathDiv")) {
			return test_math_ast(n.getOperand(0))
				/ test_math_ast(n.getOperand(1));
		} else {
			System.out.println("opérateur inconnu "+n.getOperator());
			return 0;
		}
	}

	static void dump_node_rec(AstNode n, String ofs) {
		if(n==null) {
			System.out.println(ofs+"#nil");
		} else if(n.isString()) {
			System.out.println(ofs+n.getString());
		} else {
			System.out.println(ofs+"  [");
			/* n.isList()==true */
			for(int i=0;i<=n.getOperandCount();i++) {
				dump_node_rec(n.getOperand(i),ofs+"    ");
			}
			System.out.println(ofs+"  ]");
		}
	}

	/* proxy pour dump_node_rec */
	static void dump_nodes(AstNode n) {
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
