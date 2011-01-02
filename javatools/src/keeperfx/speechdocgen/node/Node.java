package keeperfx.speechdocgen.node;

import java.io.PrintStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.IdentityHashMap;

public interface Node {
	//Node management =========================================================
	
	void setParent(ArrayNode n);
	ArrayNode getParent();
	
	//Analysis ================================================================
	
	/**
	 * Collects reference targets and reference nodes. The actual resolution is
	 * delayed, due to concurrent modification that otherwise occurs during rewrite.
	 */
	void resolveReferences(HashMap<String, ArrayNode> targets, ArrayList<ReferenceNode> refs);
	
	/**
	 * Traverses graph and marks reachable nodes as visited. Rules that are not top level
	 * die if they can't be reached in another way.
	 * @param visited
	 */
	void deadNodeIdentification(IdentityHashMap<Node, Object> visited);
	
	/**
	 * Kills off unreachable nodes.
	 * @param visited
	 */
	void deadNodeElimination(IdentityHashMap<Node, Object> visited);
	
	//Formatting ==============================================================
	
	/**
	 * Determines if we should prefer to print parenthesis around the expression this node
	 * represents.
	 */
	boolean requiresParenthesis();
	
	int lineWeight();
	
	/**
	 * Prints output suitable for a Google Code Wiki.
	 * @param stream
	 */
	void printWiki(PrintStream stream);
}
