package keeperfx.speechdocgen.node;

import java.io.PrintStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.IdentityHashMap;


public class GrammarNode extends ArrayNode {
	private static final long serialVersionUID = 5068672605195428080L;

	@Override
	public void printWiki(PrintStream stream) {
		stream.println("= Grammar =");
		for (Node n : this) {
			n.printWiki(stream);
		}
		
		stream.println("----");
	}

	public void analyze() {
		HashMap<String, ArrayNode> targets = new HashMap<String, ArrayNode>();
		ArrayList<ReferenceNode> refs = new ArrayList<ReferenceNode>();
		resolveReferences(targets, refs);
		
		for (ReferenceNode n : refs) {
			n.resolve(targets);
		}
		
		IdentityHashMap<Node, Object> visited = new IdentityHashMap<Node, Object>();
		deadNodeIdentification(visited);
		deadNodeElimination(visited);
	}
}
