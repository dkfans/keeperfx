package keeperfx.speechdocgen.node;

import java.io.PrintStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.IdentityHashMap;

public abstract class ArrayNode extends ArrayList<Node> implements Node {
	private static final long serialVersionUID = 2164078339500726900L;
	protected static final int WEIGHT_LIMIT = 30;
	
	private ArrayNode parent;
	private String data = ""; //data efter any children (but during tree construction this data can be promoted to child in add() )

	public void addCrudeXmlData(char data) {
		//ugly condition due to how XML SAX parser works - basically we want to
		//avoid excess white space
		if (Character.isWhitespace(data)) {
			if (this.data.length() == 0 ||
					Character.isWhitespace(this.data.charAt(this.data.length() - 1))) {
				return;
			}
		}

		this.data += data; //not to efficient but probably not an issue given intended domain
	}

	@Override
	public boolean add(Node n) {
		if (!data.equals("")) {
			DataNode dataNode = new DataNode(data);
			dataNode.setParent(this);
			super.add(dataNode);
			data = "";
		}
		
		n.setParent(this);
		return super.add(n);
	}

	@Override
	public void printWiki(PrintStream stream) {
		int weight = 0;
		
		for (int i = 0; i < size() - 1; ++i) {
			get(i).printWiki(stream);
			
			weight += get(i).lineWeight();
			if (weight > WEIGHT_LIMIT) {
				stream.println();
				weight = 0;
			}
			else {
				stream.print(' ');
			}
		}
		
		if (!isEmpty()) {
			get(size() - 1).printWiki(stream);
		}
		
		stream.print(data);
	}
	
	protected String getData() {
		return data;
	}
	
	public boolean requiresParenthesis() {
		for (Node n : this) {
			if (n.requiresParenthesis()) {
				return true;
			}
		}
		
		int size = size();
		return (data.equals("") &&  size >= 1) || size >= 2;
	}
	
	@Override
	public void deadNodeIdentification(IdentityHashMap<Node, Object> visited) {
		visited.put(this, this);
		
		for (Node n : this) {
			n.deadNodeIdentification(visited);
		}
	}

	@Override
	public void deadNodeElimination(IdentityHashMap<Node, Object> visited) {
		for (int i = size() - 1; i >= 0; --i) {
			if (!visited.containsKey(get(i))) {
				remove(i);
			}
		}
	}
	
	@Override
	public void resolveReferences(HashMap<String, ArrayNode> targets,
			ArrayList<ReferenceNode> refs) {
		for (Node n : this) {
			n.resolveReferences(targets, refs);
		}
	}
	
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
		int weight = data.length();
		for (Node n : this) {
			weight += n.lineWeight();
		}
		
		return weight;
	}
}
