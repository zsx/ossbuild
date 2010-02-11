/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package ossbuild.extract;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import org.w3c.dom.Document;
import org.xml.sax.SAXException;

/**
 * Reads a simple XML file that describes files to be extracted.
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public class Descriptor {
	//<editor-fold defaultstate="collapsed" desc="Variables">
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Initialization">
	public Descriptor(InputStream XMLData) throws ParserConfigurationException, SAXException, IOException {
		init(XMLData);
	}

	protected void init(InputStream XMLData) throws ParserConfigurationException, SAXException, IOException {
		DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
		DocumentBuilder builder = factory.newDocumentBuilder();
		read(builder.parse(XMLData));
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="The Meat">
	protected void read(Document document) {
		//Collapse whitespace nodes
		document.normalize();

		//Parse document
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Public Static Methods">
	public static final Descriptor Parse(InputStream XMLData) {
		try {
			return new Descriptor(XMLData);
		} catch(Throwable t) {
			return null;
		}
	}

	public static final Descriptor Parse(File XMLFile) {
		FileInputStream fis = null;
		try {
			return Parse((fis = new FileInputStream(XMLFile)));
		} catch(Throwable t) {
			return null;
		} finally {
			try {
				if (fis != null)
					fis.close();
			} catch(IOException ie) {
			}
		}
	}
	//</editor-fold>
}
