
package ossbuild.extract;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathException;
import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import ossbuild.Path;
import ossbuild.StringUtil;
import ossbuild.Sys;
import static ossbuild.extract.ResourceUtils.*;

/**
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public class Package implements IResourcePackage {
	//<editor-fold defaultstate="collapsed" desc="Constants">
	public static final String
		  ATTRIBUTE_PACKAGE		= "package"
		, ATTRIBUTE_DIRECTORY	= "directory"
	;
	//</editor-fold>
	
	//<editor-fold defaultstate="collapsed" desc="Variables">
	protected long totalSize = 0L;
	protected String name, directory;
	protected String resourcePrefix;
	protected File destDirectory;
	protected List<IResourceProcessor> processors;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Initialization">
	public Package(final ResourceProcessorFactory processorFactory, final IVariableProcessor variableProcessor, final Node node, final XPath xpath, final Document document) throws XPathException {
		initForXML(processorFactory, variableProcessor, node, xpath, document);
	}
	
	public Package(final String Name, final String Directory) {
		initVars(Name, Directory);
	}

	public Package(final String Name, final String Directory, final IResourceProcessor... Processors) {
		initVars(Name, Directory, Processors);
	}

	private void initProcessors(final IResourceProcessor... Processors) {
		if (Processors == null || Processors.length <= 0)
			return;
		for(IResourceProcessor p : Processors) {
			if (p == null)
				continue;
			addResourceProcessor(p);
		}
	}

	private void initVars(final String Name, final String Directory, final IResourceProcessor... Processors) {
		init();
		this.name = Name;
		this.directory = Directory;
		configure();
		initProcessors(Processors);
		initAfter();
	}

	private void initForXML(final ResourceProcessorFactory processorFactory, final IVariableProcessor variableProcessor, final Node node, final XPath xpath, final Document document) throws XPathException {
		init();
		read(processorFactory, variableProcessor, node, xpath, document);
		initAfter();
	}

	@SuppressWarnings("unchecked")
	private void init() {
		this.processors = new ArrayList(10);
	}

	private void initAfter() {
		calculateTotals();
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Getters">
	@Override
	public int getTotalResourceCount() {
		return processors.size();
	}
	
	@Override
	public long getTotalSize() {
		return totalSize;
	}
	
	@Override
	public String getName() {
		return name;
	}

	@Override
	public String getDirectory() {
		return directory;
	}

	@Override
	public IResourceProcessor[] getResourceProcessors() {
		return processors.toArray(new IResourceProcessor[processors.size()]);
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Iterable Methods">
	@Override
	public Iterator<IResourceProcessor> iterator() {
		return processors.iterator();
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Visit">
	@Override
	public void visit(IVisitor visitor) {
		if (visitor == null)
			return;
		for(IResourceProcessor p : processors)
			if (!visitor.each(this, p))
				return;
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Helper Methods">
	@Override
	public void calculateTotals() {
		totalSize = 0L;
		
		//Calculate total size (in bytes) of all the resources
		for(IResourceProcessor p : processors) {
			if (p == null)
				continue;
			if (p.supportsSize())
				totalSize += p.getSize();
		}
	}

	protected void configure() {
		//Produce resource path
		//Takes "resources.extraction" and turns it into "/resources/extraction/"
		//for use in appending resource names later on.
		resourcePrefix = Sys.createPackageResourcePrefix(name);
		destDirectory = new File(directory);
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Public Methods">
	public boolean addResourceProcessor(final IResourceProcessor Processor) {
		if (Processor == null)
			return false;
		processors.add(Processor);
		if (Processor.supportsSize())
			totalSize += Processor.getSize();
		return true;
	}

	public boolean removeResourceProcessor(final IResourceProcessor Processor) {
		if (Processor == null || processors.isEmpty() || !processors.contains(Processor))
			return false;
		processors.remove(Processor);
		if (Processor.supportsSize())
			totalSize -= Processor.getSize();
		return true;
	}

	public boolean clearResourceProcessors() {
		processors.clear();
		totalSize = 0L;
		return true;
	}

	@Override
	public String resourcePath(final String ResourceName) {
		if (StringUtil.isNullOrEmpty(ResourceName))
			return StringUtil.empty;
		return resourcePrefix + ResourceName;
	}

	@Override
	public File filePath(final String FileName) {
		return Path.combine(destDirectory, FileName);
	}

	@Override
	public File filePath(final String SubDirectory, final String FileName) {
		return Path.combine(Path.combine(destDirectory, SubDirectory), FileName);
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Public Static Methods">
	//<editor-fold defaultstate="collapsed" desc="newInstance">
	public static Package newInstance(final ResourceProcessorFactory ProcessorFactory, final IVariableProcessor VariableProcessor, final Node Node, final XPath XPath, final Document Document) throws XPathException {
		return new Package(ProcessorFactory, VariableProcessor, Node, XPath, Document);
	}

	public static Package newInstance(final String PackageName, final File Directory) {
		try {
			return newInstance(PackageName, Directory.getCanonicalPath());
		} catch(IOException ie) {
			throw new RuntimeException("Error creating new Package.", ie);
		}
	}

	public static Package newInstance(final String PackageName, final File Directory, final IResourceProcessor... Processors) {
		try {
			return newInstance(PackageName, Directory.getCanonicalPath(), Processors);
		} catch(IOException ie) {
			throw new RuntimeException("Error creating new Package.", ie);
		}
	}

	public static Package newInstance(final String PackageName, final String Directory) {
		return new Package(PackageName, Directory);
	}

	public static Package newInstance(final String PackageName, final String Directory, final IResourceProcessor... Processors) {
		return new Package(PackageName, Directory, Processors);
	}
	//</editor-fold>
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="The Meat">
	protected void read(final ResourceProcessorFactory processorFactory, final IVariableProcessor variableProcessor, final Node node, final XPath xpath, final Document document) throws XPathException  {
		//Read attribute values in
		this.name = stringAttributeValue(variableProcessor, StringUtil.empty, node, ATTRIBUTE_PACKAGE);
		this.directory = stringAttributeValue(variableProcessor, StringUtil.empty, node, ATTRIBUTE_DIRECTORY);

		//Set variables on a per-package basis
		variableProcessor.saveVariable(VariableProcessorFactory.VAR_PKG, name);
		variableProcessor.saveVariable(VariableProcessorFactory.VAR_DIR, new File(directory).getAbsolutePath());

		configure();

		NodeList lst;
		Node childNode;
		IResourceProcessor processor;

		if ((lst = (NodeList)xpath.evaluate("*", node, XPathConstants.NODESET)) == null || lst.getLength() <= 0)
			return;

		//Iterate over every child tag (e.g. <Library />, <File />), extract its name, attempt to
		//create a resource processor, and then add it to our list.
		for(int i = 0; i < lst.getLength() && (childNode = lst.item(i)) != null; ++i) {
			if ((processor = processorFactory.createProcessor(childNode.getNodeName())) != null) {
				if (processor.load(this, xpath, childNode, variableProcessor))
					processors.add(processor);
			}
		}
	}
	//</editor-fold>
}
