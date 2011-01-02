package keeperfx.speechdocgen;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.PrintStream;

import keeperfx.speechdocgen.node.GrammarNode;

import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.xml.sax.XMLReader;
import org.xml.sax.helpers.XMLReaderFactory;

public class SpeechDocGen {
	/**
	 * Utility which converts a subset of a MS Speech grammar XML file to readable
	 * documentation which describes the voice commands the grammar specifies.
	 * @param args First argument must be the filename of the output file.
	 * 	One or more remaining arguments are input files (.xml grammars).
	 * @throws SAXException 
	 * @throws IOException 
	 */
	public static void main(String[] args) throws SAXException, IOException {
		if (args.length < 2) {
			System.out.println("Usage: output_file input_file+");
			return;
		}
		
		XMLReader reader = XMLReaderFactory.createXMLReader();
		GrammarHandler handler = new GrammarHandler();
		reader.setContentHandler(handler);
		reader.setErrorHandler(handler);
		reader.setProperty("http://xml.org/sax/properties/lexical-handler", handler);
		
		for (int i = 1; i < args.length; ++i) {
			InputSource inputSource = new InputSource(args[i]);
			reader.parse(inputSource);
		}
		
		File outputFile = new File(args[0]);
		PrintStream stream = new PrintStream(new FileOutputStream(outputFile));
		
		for (GrammarNode grammar : handler.getGrammars()) {
			grammar.analyze();
			grammar.printWiki(stream);
		}
		
		printWikiLegend(stream);
		stream.close();
	}

	private static void printWikiLegend(PrintStream stream) {
		stream.println("= Legend =");
		stream.println("{{{");
		stream.println("Forced precedence: (like this)");
		stream.println("Option: [like this] or (like this)?");
		stream.println("List: like | this");
		stream.println("}}}");
	}
}
