package keeperfx.speechdocgen.node;

import java.io.PrintStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.IdentityHashMap;

public class RuleNode extends ArrayNode {
	private static final long serialVersionUID = 926251384177075839L;
	private final String id;
	private boolean topLevel;
	private String title;
	private String description;

	public RuleNode(String id, boolean topLevel, String docComment) {
		this.id = id;
		this.topLevel = topLevel;
		
		String[] fields = docComment.split("@");
		for (String f : fields) {
			if (f.startsWith("title")) {
				title = f.substring("title".length()).trim();
			}
			else if (f.startsWith("description")) {
				description = f.substring("description".length()).trim();
			}
		}
	}
	
	public String getIdentifier() {
		return id;
	}

	@Override
	public void printWiki(PrintStream stream) {
		if (title == null) {
			stream.println("== Rule " + id + " ==");
		}
		else {
			stream.println("== " + title + " ==");
		}
		
		if (description != null) {
			stream.println(description);
		}
		
		stream.println("==== Syntax: ====");
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
