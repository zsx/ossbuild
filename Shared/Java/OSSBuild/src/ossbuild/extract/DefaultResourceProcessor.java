
package ossbuild.extract;

import java.io.File;
import org.w3c.dom.Node;
import ossbuild.StringUtil;

/**
 * Default implementation of a resource processor.
 * 
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public abstract class DefaultResourceProcessor implements IResourceProcessor {
	//<editor-fold defaultstate="collapsed" desc="Constants">
	public static final String
		  ATTRIBUTE_TITLE			= "title"
		, ATTRIBUTE_TRANSIENT		= "transient"
		, ATTRIBUTE_DEST_NAME		= "destName"
		, ATTRIBUTE_RESOURCE_NAME	= "resource"
		, ATTRIBUTE_DESCRIPTION		= "description" 
		, ATTRIBUTE_SUBDIRECTORY	= "subDirectory"
	;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Variables">
	protected long size = 0L;
	protected boolean shouldBeTransient;
	protected String resourceName, subDirectory, destName, title, description;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Initialization">
	public DefaultResourceProcessor() {
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Getters">
	public boolean supportsSize() {
		return Utils.supportsSize(getClass());
	}

	public long getSize() {
		return size;
	}
	
	public boolean isTransient() {
		return shouldBeTransient;
	}

	public String getResourceName() {
		return resourceName;
	}

	public String getSubDirectory() {
		return subDirectory;
	}

	public String getDestName() {
		return destName;
	}

	public String getTitle() {
		return title;
	}

	public String getDescription() {
		return description;
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Helper Methods">
	protected String expandVariables(String value) {
		return Utils.expandVariables(value);
	}
	
	protected String valueForAttribute(Node node, String name) {
		return Utils.valueForAttribute(node, name);
	}

	protected String stringAttributeValue(String defaultValue, Node node, String name) {
		return Utils.stringAttributeValue(defaultValue, node, name);
	}

	protected boolean boolAttributeValue(boolean defaultValue, Node node, String name) {
		return Utils.boolAttributeValue(defaultValue, node, name);
	}

	protected File fileAttributeValue(String defaultValue, Node node, String name) {
		return Utils.fileAttributeValue(defaultValue, node, name);
	}

	protected File fileAttributeValue(File defaultValue, Node node, String name) {
		return Utils.fileAttributeValue(defaultValue, node, name);
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="ToString">
	@Override
	public String toString() {
		return
			getClass().getSimpleName() +
			"\n    Name:\t\t"       + resourceName      +
			"\n    Title:\t\t"      + title             +
			"\n    SubDirectory:\t" + subDirectory      +
			"\n    DestName:\t"     + destName          +
			"\n    Transient:\t\t"  + shouldBeTransient +
			"\n    Description:\t"  + description
		;
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="IResourceProcessor Methods">
	public boolean load(final Node node) {
		if (node != null) {
			//Read attribute values in
			this.resourceName = stringAttributeValue(StringUtil.empty, node, ATTRIBUTE_RESOURCE_NAME);
			this.description = stringAttributeValue(StringUtil.empty, node, ATTRIBUTE_DESCRIPTION);
			this.subDirectory = stringAttributeValue(StringUtil.empty, node, ATTRIBUTE_SUBDIRECTORY);
			this.destName = stringAttributeValue(StringUtil.empty, node, ATTRIBUTE_DEST_NAME);
			this.shouldBeTransient = boolAttributeValue(true, node, ATTRIBUTE_TRANSIENT);

			//Default this to be the resource name if no title is found
			this.title = stringAttributeValue(this.resourceName, node, ATTRIBUTE_TITLE);
		}

		//Let subclasses parse any add'l settings they need
		if (loadSettings(node)) {
			//Ask for the size if settings were successfully loaded
			size = requestSize();
			return true;
		}

		return false;
	}

	public boolean process() {
		return processResource();
	}
	//</editor-fold>

	protected long requestSize() { return size; }
	protected abstract boolean loadSettings(final Node node);
	protected abstract boolean processResource();
}
