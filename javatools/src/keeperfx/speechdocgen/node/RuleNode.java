package keeperfx.speechdocgen.node;

import java.io.PrintStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.IdentityHashMap;

public class RuleNode extends ArrayNode {
	private static final long serialVersionUID = 926251384177075839L;
	private final String id;
	private boolean topLevel;

	public RuleNode(String id, boolean topLevel) {
		this.id = id;
		this.topLevel = topLevel;
	}
	
	public String getIdentifier() {
		return id;
	}

	@Override
	public void printWiki(PrintStream stream) {
		stream.println("== Rule " + id + " ==");
		stream.println("{{{");
		super.printWiki(stream);
		stream.println("\n}}}");
		stream.println();
	}
	
	@Override
	public void resolveReferences(HashMap<String, ArrayNode> targets,
			ArrayList<ReferenceNode> refs) {
		targets.put(id, this);
		super.resolveReferences(targets, refs);
	}

	@Override
	public void deadNodeIdentification(IdentityHashMap<Node, Object> visited) {
		if (topLevel) { //otherwise, there's no live arc from parent to this
			super.deadNodeIdentification(visited);
		}
	}
}
