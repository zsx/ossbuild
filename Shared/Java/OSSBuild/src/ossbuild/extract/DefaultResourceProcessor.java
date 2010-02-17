
package ossbuild.extract;

import java.io.File;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathException;
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

	public DefaultResourceProcessor(boolean IsTransient, String ResourceName, String SubDirectory, String DestName, String Title, String Description) {
		if (StringUtil.isNullOrEmpty(ResourceName))
			throw new NullPointerException("Missing ResourceName");

		this.shouldBeTransient = IsTransient;
		this.resourceName = ResourceName;
		this.subDirectory = SubDirectory;
		this.destName = !StringUtil.isNullOrEmpty(DestName) ? DestName : ResourceName;
		this.title = !StringUtil.isNullOrEmpty(Title) ? Title : DestName;
		this.description = Description;
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Getters">
	public boolean supportsSize() {
		return ResourceUtils.supportsSize(getClass());
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

	public String getName() {
		return resourceName;
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Helper Methods">
	protected String expandVariables(String value) {
		return ResourceUtils.expandVariables(value);
	}
	
	protected String valueForAttribute(Node node, String name) {
		return ResourceUtils.valueForAttribute(node, name);
	}

	protected String stringAttributeValue(String defaultValue, Node node, String name) {
		return ResourceUtils.stringAttributeValue(defaultValue, node, name);
	}

	protected boolean boolAttributeValue(boolean defaultValue, Node node, String name) {
		return ResourceUtils.boolAttributeValue(defaultValue, node, name);
	}

	protected File fileAttributeValue(String defaultValue, Node node, String name) {
		return ResourceUtils.fileAttributeValue(defaultValue, node, name);
	}

	protected File fileAttributeValue(File defaultValue, Node node, String name) {
		return ResourceUtils.fileAttributeValue(defaultValue, node, name);
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
	@Override
	public boolean load(final IResourcePackage pkg, final XPath xpath, final Node node) throws XPathException {
		if (node != null) {
			//Read attribute values in
			this.resourceName = stringAttributeValue(StringUtil.empty, node, ATTRIBUTE_RESOURCE_NAME);
			this.description = stringAttributeValue(StringUtil.empty, node, ATTRIBUTE_DESCRIPTION);
			this.subDirectory = stringAttributeValue(StringUtil.empty, node, ATTRIBUTE_SUBDIRECTORY);
			this.shouldBeTransient = boolAttributeValue(true, node, ATTRIBUTE_TRANSIENT);

			//Default this to be the destName/resource name if no title/destName is found
			this.destName = stringAttributeValue(this.resourceName, node, ATTRIBUTE_DEST_NAME);
			this.title = stringAttributeValue(this.destName, node, ATTRIBUTE_TITLE);
		}

		//Let subclasses parse any add'l settings they need
		if (loadSettings(pkg.resourcePath(resourceName), pkg, xpath, node)) {
			//Ask for the size if settings were successfully loaded
			size = requestSize();
			return true;
		}

		return false;
	}

	@Override
	public boolean process(final String fullResourceName, final IResourcePackage pkg, final IResourceProgressListener progress) {
		return processResource(fullResourceName, pkg, progress);
	}
	//</editor-fold>

	protected long requestSize() { return size; }
	protected abstract boolean loadSettings(final String fullResourceName, final IResourcePackage pkg, final XPath xpath, final Node node) throws XPathException;
	protected abstract boolean processResource(final String fullResourceName, final IResourcePackage pkg, final IResourceProgressListener progress);
}
