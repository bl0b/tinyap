package tinyajp;

class Native {
	static native long	parserNew();

	static native String	parserGetWhitespace(long p_hndl);
	static native void	parserSetWhitespace(long p_hndl,String ws);
	static native void	parserSetWhitespaceRegexp(long p_hndl,String wsre);

	static native String	parserGetGrammar(long p_hndl);
	static native void	parserSetGrammar(long p_hndl,String g);
	static native long	parserGetGrammarAst(long p_hndl);
	static native void	parserSetGrammarAst(long p_hndl,long gast);

	static native String	parserGetSourceFile(long p_hndl);
	static native String	parserGetSourceBuffer(long p_hndl);
	static native void	parserSetSourceFile(long p_hndl,String fnam);
	static native void	parserSetSourceBuffer(long p_hndl,String buf);

	static native boolean	parserParse(long p_hndl);
	static native boolean	parserParseAsGrammar(long p_hndl);
	static native boolean	parserParsedOK(long p_hndl);

	static native long	parserGetOutput(long p_hndl);

	static native int	parserGetErrorCol(long p_hndl);
	static native int	parserGetErrorRow(long p_hndl);
	static native String	parserGetError(long p_hndl);

	static native void	parserDelete(long p_hndl);

	static native boolean	nodeIsNil(long n_hndl);

	static native boolean	nodeIsList(long n_hndl);
	static native long	listGetElement(long n_hndl,int i);

	static native boolean	nodeIsString(long n_hndl);
	static native String	nodeGetString(long n_hndl);

	static        boolean	nodeIsOp(long n_hndl) { return nodeIsList(n_hndl)&&nodeIsString(listGetElement(n_hndl,0)); }
	static native String	nodeGetOperator(long n_hndl);
	static native long	nodeGetOperand(long n_hndl,int i);
	static native int	nodeGetOperandCount(long n_hndl);
	
	static native String	nodeToString(long n_hndl);
	static native void	nodeToFile(long n_hndl,String fnam);

	static native int	nodeGetRow(long n_hndl);
	static native int	nodeGetCol(long n_hndl);

	static {
		System.loadLibrary("tinyajp_Native");
	}
}
