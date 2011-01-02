package keeperfx.speechdocgen.node;

import java.io.PrintStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.IdentityHashMap;

public class DataNode implements Node {
	private ArrayNode parent;
	private final String data;

	public DataNode(String data) {
		this.data = data;
	}

	@Override
	public void printWiki(PrintStream stream) {
		stream.print(data);
	}

	@Override
	public boolean requiresParenthesis() {
		return false;
	}

	@Override
	public void deadNodeElimination(IdentityHashMap<Node, Object> visited) {}

	@Override
	public void deadNodeIdentification(IdentityHashMap<Node, Object> visited) {
		visited.put(this, this);
	}

	@Override
	public void resolveReferences(HashMap<String, ArrayNode> targets,
			ArrayList<ReferenceNode> refs) {}

	@Override
	public ArrayNode getParent() {
		return parent;
	}

	@Override
	public void setParent(ArrayNode n) {
		parent = n;
	}

	@Override
	public int lineWeight() {
		return data.length();
	}
}
