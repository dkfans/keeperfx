package keeperfx.speechdocgen.node;

import java.io.PrintStream;

public class OptionNode extends ArrayNode {
	private static final long serialVersionUID = 8601947079685819483L;

	@Override
	public void printWiki(PrintStream stream) {
		if (requiresParenthesis()) {
			stream.print("[");
		}
		
		super.printWiki(stream);
		
		if (requiresParenthesis()) {
			stream.print("]");
		}
		else {
			stream.print('?');
		}
	}
}
