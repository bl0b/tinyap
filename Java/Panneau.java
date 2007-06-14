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
import java.lang.*;
import java.awt.*;
import javax.swing.*;
import javax.swing.tree.*;
import java.awt.event.*;

public class Panneau extends JPanel {
	private JTree resultAST;
	private JTextArea result;
	private DefaultMutableTreeNode astRoot;
	private JTextArea grammar;
	private JTextField input;
	private JPanel resultPanel;
	private JSplitPane splitPane;
	private Font font;

	private String prettyNode(AstNode n) {
		String name;
		if(n.isString()) {
			name=n.getString();
		} else if(n.isList()) {
			name=n.getElementAt(0).getString();
		} else {
			name="oups";
		}
		return name+" ["+n.getRow()+","+n.getCol()+"]";
	}

	private class HeadPhones implements ActionListener {
		public void actionPerformed(ActionEvent ae) {
			String expr;

			expr=Panneau.this.input.getText();
			if(expr.equals("quit")) {
				System.exit(0);
			}

			Parser parser=new Parser();

			parser.setGrammar("CamelCasing")
			      .setSourceBuffer(Panneau.this.grammar.getText())
			      .parseAsGrammar();

			if(!parser.parsedOK()) {
				Panneau.this.showError(parser.getErrorRow(),parser.getErrorCol(),parser.getError());
				return;
			}

			parser.setSourceBuffer(expr).parse();

			if(!parser.parsedOK()) {
				Panneau.this.showError(parser.getErrorRow(),parser.getErrorCol(),parser.getError());
				return;
			} else {
				AstNode n;
				double d;
				String resultats=new String();
				for(int i=0;i<parser.getOutput().getSize();i++) {
					n=parser.getOutput().getElementAt(i);
					if(n!=null) {
						d=Interpreter.test_math_ast(n);
						//dump_nodes(n);
						//System.out.println(" = "+d);
						resultats+=" = "+d+"\n";
						
					}
				}
				Panneau.this.showResult(parser.getOutput(),resultats);
			}
		}
	}

	public Panneau() {
		font=new Font("courier",0,12);
		JPanel inputPanel = new JPanel();
		setLayout(new BorderLayout());
		
		//resultPanel.setPreferredSize(new Dimension(350,500));
		
		grammar = new JTextArea();
		grammar.setFont(font);
		//grammar.setSize(300,100);

		input = new JTextField();

		inputPanel.setLayout(new BorderLayout());
		inputPanel.add(input,BorderLayout.SOUTH);
		inputPanel.add(new JLabel(
				"Enter an expression that will be parsed according to the above grammar, or 'quit' to quit"),
				BorderLayout.NORTH);

		input.addActionListener(new HeadPhones());

		//add(grammar,BorderLayout.NORTH);
		splitPane = new JSplitPane(
				JSplitPane.HORIZONTAL_SPLIT,
				new JPanel(), new JScrollPane(grammar));
		add(splitPane,BorderLayout.CENTER);
		//add(new JScrollPane(resultAST),BorderLayout.CENTER);
		//add(new JScrollPane(grammar),BorderLayout.EAST);
		add(inputPanel,BorderLayout.SOUTH);
		setVisible(true);

		setPreferredSize(new Dimension(800,600));
	}

	public void setGrammar(String g) {
		grammar.setText(g);
	}

	private void showRes_rec(DefaultMutableTreeNode p,AstNode n) {
		if(n.isString()) {
			p.add(new DefaultMutableTreeNode(prettyNode(n)));
		} else if(n.isList()) {
			DefaultMutableTreeNode op;
			if(n.getElementAt(0).isString()) {
				op=new DefaultMutableTreeNode(prettyNode(n));
				//System.out.println("w/ op");
			} else {
				op=new DefaultMutableTreeNode(new String("<nop>"));
				//System.out.println("w/o op");
			}
			//System.out.println("n.size = "+n.getSize());
			for(int i=1;i<n.getSize();i++) {
				showRes_rec(op,n.getElementAt(i));
			}
			p.add(op);
		} else if(n.isNil()) {
			p.add(new DefaultMutableTreeNode(new String("(nil)")));
		} else {
			System.out.println("oups");
		}
	}
	
	public void showResult(AstNode n,String res) {
		astRoot=new DefaultMutableTreeNode(new String("Result"));
		resultAST=new JTree(astRoot);
		result = new JTextArea();
		result.setEditable(false);
		result.setFont(font);

		//resultPanel.remove(resultAST);
		if(!(n==null||n.isNil())) {
			for(int i=0;i<n.getSize();i++) {
				showRes_rec(astRoot,n.getElementAt(i));
			}
		}
		result.setText(input.getText()+"\n"+res);
		result.repaint();
		input.setText("");
		resultPanel = new JPanel();
		resultPanel.setLayout(new BorderLayout());
		resultPanel.add(new JScrollPane(resultAST),BorderLayout.CENTER);
		resultPanel.add(result,BorderLayout.SOUTH);
		splitPane.setLeftComponent(resultPanel);
		resultPanel.repaint();
	}

	public void showError(int l,int c,String err) {
		showResult(AstNode.nil,"Parser : error at line "+l+", column "+c+" :\n"+err);
	}
}

