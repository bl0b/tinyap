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
import java.lang.*;
import java.awt.*;
import javax.swing.*;
import javax.swing.tree.*;
import java.awt.event.*;

import tinyajp.AstNode;
import tinyajp.Parser;

public class Demo {
	static public void main(String[] args) {
		Panneau demo=new Panneau();
		JFrame frame=new JFrame("TinyaJP demo");
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

		frame.setContentPane(demo);
		frame.setSize(400,600);
		frame.pack();
		
		try {
			java.io.FileReader fGram=new java.io.FileReader("math.CC.gram");
			String cbuf=new String();
			int c=0;
		       	while((c=fGram.read())!=-1) cbuf+=(char)c;
			demo.setGrammar(cbuf);
		} catch(java.io.IOException e) {}
		frame.setVisible(true);
	}
}
