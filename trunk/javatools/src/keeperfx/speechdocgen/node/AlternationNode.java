package keeperfx.speechdocgen.node;

import java.io.PrintStream;

public class AlternationNode extends ArrayNode {
	private static final long serialVersionUID = -5569446306461877368L;

	@Override
	public void printWiki(PrintStream stream) {
		int weight = 0;
		
		if (!(getParent() instanceof RuleNode) && //ugly but effective for the moment
				getParent().size() > 1) {
			stream.print("( ");
		}
		
		for (int i = 0; i < size(); ++i) {
			Node n = get(i);
			
			if (n.requiresParenthesis()) {
				stream.print('(');
			}
			
			n.printWiki(stream);
			
			if (n.requiresParenthesis()) {
				stream.print(')');
			}
			
			if (i != size() - 1) {
				stream.print(" | ");
				
				weight += n.lineWeight();
				if (weight > WEIGHT_LIMIT) {
					stream.println();
					weight = 0;
				}
			}
		}
		
		if (!(getParent() instanceof RuleNode) &&
				getParent().size() > 1) {
			stream.print(" )");
		}
	}
}
