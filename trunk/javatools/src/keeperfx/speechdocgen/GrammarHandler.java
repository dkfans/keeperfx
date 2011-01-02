package keeperfx.speechdocgen;

import java.util.ArrayList;
import java.util.Stack;

import keeperfx.speechdocgen.node.AlternationNode;
import keeperfx.speechdocgen.node.GrammarNode;
import keeperfx.speechdocgen.node.ArrayNode;
import keeperfx.speechdocgen.node.OptionNode;
import keeperfx.speechdocgen.node.PhraseNode;
import keeperfx.speechdocgen.node.ReferenceNode;
import keeperfx.speechdocgen.node.RuleNode;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.ext.LexicalHandler;
import org.xml.sax.helpers.DefaultHandler;

public class GrammarHandler extends DefaultHandler implements LexicalHandler {
	private final ArrayList<GrammarNode> grammars;
	
	//parse state
	private final Stack<ArrayNode> stack;
	private String latestDocComment = "";

	public GrammarHandler() {
		grammars = new ArrayList<GrammarNode>();
		stack = new Stack<ArrayNode>();
	}
	
	public ArrayList<GrammarNode> getGrammars() {
		return grammars;
	}

	@Override
	public void startDocument() {
		stack.clear();
	}
	
	@Override
	public void endDocument() {
		assert(stack.isEmpty());
	}

	@Override
	public void startElement(String uri, String localName, String qName,
			Attributes attributes) throws SAXException {
		if (localName.equals("P")) {
			handlePhrase(attributes);
		}
		else if (localName.equals("O")) {
			handleOptional(attributes);
		}
		else if (localName.equals("L")) {
			handleList(attributes);
		}
		else if (localName.equals("RULEREF")) {
			handleRuleref(attributes);
		}
		else if (localName.equals("RULE")) {
			handleRule(attributes);
		}
		else if (localName.equals("GRAMMAR")) {
			handleGrammar(attributes);
		}
		
		latestDocComment = "";
	}
	
	@Override
	public void endElement(String uri, String localName, String qName)
			throws SAXException {
		if (	localName.equals("P") ||
				localName.equals("O") ||
				localName.equals("L") ||
				localName.equals("RULEREF") ||
				localName.equals("RULE") ||
				localName.equals("GRAMMAR")) {
			stack.pop();
		}
	}

	@Override
	public void characters(char[] ch, int start, int length)
			throws SAXException {
		if (!stack.isEmpty()) {
			for (int i = start, end = start + length; i < end; ++i) {
				if (Character.isWhitespace(ch[i]) && ch[i] != ' ') {
					continue;
				}
				
				stack.lastElement().addCrudeXmlData(ch[i]);
			}
		}
	}
	
	private void handlePhrase(Attributes attributes) {
		pushNode(new PhraseNode());
	}

	private void handleOptional(Attributes attributes) {
		pushNode(new OptionNode());
	}

	private void handleList(Attributes attributes) {
		pushNode(new AlternationNode());
	}
	
	private void handleRuleref(Attributes attributes) {
		String refid = attributes.getValue("", "REFID");
		pushNode(new ReferenceNode(refid));
	}

	private void handleRule(Attributes attributes) {
		String id = attributes.getValue("", "ID");
		boolean topLevel = "ACTIVE".equals(attributes.getValue("", "TOPLEVEL"));
		pushNode(new RuleNode(id, topLevel, latestDocComment));
	}
	
	private void handleGrammar(Attributes attributes) {
		assert(stack.isEmpty());
		
		GrammarNode n = new GrammarNode();
		stack.push(n);
		grammars.add(n);
	}

	private void pushNode(ArrayNode n) {
		assert(!stack.isEmpty());
		stack.lastElement().add(n);
		stack.push(n);
	}

	@Override
	public void comment(char[] ch, int offset, int count) throws SAXException {
		if (ch[offset] == '*') {
			latestDocComment = new String(ch, offset, count);
		}
	}

	@Override
	public void endCDATA() throws SAXException {}

	@Override
	public void endDTD() throws SAXException {}

	@Override
	public void endEntity(String arg0) throws SAXException {}

	@Override
	public void startCDATA() throws SAXException {}

	@Override
	public void startDTD(String arg0, String arg1, String arg2) {}

	@Override
	public void startEntity(String arg0) throws SAXException {}
}
