
package ossbuild.extract;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CancellationException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Future;
import java.util.concurrent.RejectedExecutionException;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathException;
import javax.xml.xpath.XPathFactory;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;
import ossbuild.Namespaces;
import ossbuild.StringUtil;
import static ossbuild.extract.ResourceUtils.*;

/**
 * Reads a simple XML file that describes files to be extracted.
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public class Resources {
	//<editor-fold defaultstate="collapsed" desc="Variables">
	protected long totalResourceSize;
	protected int totalResourceCount;
	protected IResourcePackage[] packages;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Initialization">
	public Resources(IResourcePackage... Packages) {
		initFromPackages(Packages);
	}
	
	public Resources(final InputStream XMLData) throws XPathException, ParserConfigurationException, SAXException, IOException {
		initFromXML(ResourceProcessorFactory.DEFAULT_INSTANCE, XMLData);
	}

	public Resources(final ResourceProcessorFactory ProcessorFactory, final InputStream XMLData) throws XPathException, ParserConfigurationException, SAXException, IOException {
		initFromXML(ProcessorFactory, XMLData);
	}

	protected void initFromPackages(IResourcePackage[] Packages) {
		if (Packages == null)
			throw new NullPointerException("Processors cannot be null");
		
		this.packages = Packages;
		
		initAfter();
	}

	protected void initFromXML(final ResourceProcessorFactory ProcessorFactory, final InputStream XMLData) throws XPathException, ParserConfigurationException, SAXException, IOException {
		final DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
		final DocumentBuilder builder = factory.newDocumentBuilder();
		final XPathFactory xpathFactory = XPathFactory.newInstance();
		final XPath xpath = xpathFactory.newXPath();

		//Creates a context that always has "ossbuild" as a prefix -- useful for xpath evaluations
		xpath.setNamespaceContext(Namespaces.createNamespaceContext());

		this.packages = read(ProcessorFactory, xpath, builder.parse(XMLData));

		initAfter();
	}

	protected void initAfter() {
		calculateTotals();
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Helper Methods">
	protected void calculateTotals() {
		//Calculate total size and resource count
		for(IResourcePackage pkg : packages) {
			if (pkg == null)
				continue;

			totalResourceCount += pkg.getTotalResourceCount();
			totalResourceSize += pkg.getTotalSize();
		}
	}

	protected void notifyProgressBegin(IResourceProgressListener progress, long startTime) {
		if (progress != null)
			progress.begin(getTotalResourceCount(), getTotalPackageCount(), getTotalResourceSize(), startTime);
	}

	protected void notifyProgressReport(IResourceProgressListener progress, long totalBytes, int totalResources, int totalPkgs, long startTime, String message) {
		if (progress != null)
			progress.report(getTotalResourceCount(), getTotalPackageCount(), getTotalResourceSize(), totalBytes, totalResources, totalPkgs, startTime, Math.abs(System.currentTimeMillis() - startTime), message);
	}

	protected void notifyProgressError(IResourceProgressListener progress, Throwable exception, String message) {
		if (progress != null)
			progress.error(exception, message);
	}

	protected void notifyProgressEnd(IResourceProgressListener progress, boolean success, long totalBytes, int totalResources, int totalPkgs, long startTime, long endTime) {
		if (progress != null)
			progress.end(success, getTotalResourceCount(), getTotalPackageCount(), getTotalResourceSize(), totalBytes, totalResources, totalPkgs, startTime, endTime);
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Getters">
	public int getTotalPackageCount() {
		return packages.length;
	}
	
	public int getTotalResourceCount() {
		return totalResourceCount;
	}

	public long getTotalResourceSize() {
		return totalResourceSize;
	}
	
	public IResourcePackage[] getPackages() {
		return packages;
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Public Methods">
	//<editor-fold defaultstate="collapsed" desc="Overloads">
	public Future extract(final IResourceCallback callback) {
		final ExecutorService svc = createUnprivilegedExecutorService();
		try {
			return extract(packages, svc, IResourceFilter.None, IResourceProgressListener.None, callback);
		} finally {
			svc.shutdown();
		}
	}

	public Future extract(final IResourceProgressListener progress) {
		final ExecutorService svc = createUnprivilegedExecutorService();
		try {
			return extract(packages, svc, IResourceFilter.None, progress, IResourceCallback.None);
		} finally {
			svc.shutdown();
		}
	}

	public Future extract(final IResourceFilter filter) {
		final ExecutorService svc = createUnprivilegedExecutorService();
		try {
			return extract(packages, svc, filter, IResourceProgressListener.None, IResourceCallback.None);
		} finally {
			svc.shutdown();
		}
	}

	public Future extract(final IResourceFilter filter, final IResourceCallback callback) {
		final ExecutorService svc = createUnprivilegedExecutorService();
		try {
			return extract(packages, svc, filter, IResourceProgressListener.None, callback);
		} finally {
			svc.shutdown();
		}
	}

	public Future extract(final IResourceProgressListener progress, final IResourceCallback callback) {
		final ExecutorService svc = createUnprivilegedExecutorService();
		try {
			return extract(packages, svc, IResourceFilter.None, progress, callback);
		} finally {
			svc.shutdown();
		}
	}

	public Future extract(final IResourceFilter filter, final IResourceProgressListener progress, final IResourceCallback callback) {
		final ExecutorService svc = createUnprivilegedExecutorService();
		try {
			return extract(packages, svc, filter, progress, callback);
		} finally {
			svc.shutdown();
		}
	}
	//</editor-fold>

	public Future extract(final IResourcePackage[] pkgs, final ExecutorService executor, final IResourceFilter filter, final IResourceProgressListener progress, final IResourceCallback callback) {
		return executor.submit(new Runnable() {
			@Override
			public void run() {
				try {
					//<editor-fold defaultstate="collapsed" desc="Make compiler happy">
					if (false)
						throw new InterruptedException();
					//</editor-fold>

					//<editor-fold defaultstate="collapsed" desc="Prepare">
					if (callback != null)
						callback.prepare(Resources.this);
					//</editor-fold>

					//Do the real work
					extractResources(pkgs, filter, progress);

					//<editor-fold defaultstate="collapsed" desc="Completed">
					if (callback != null)
						callback.completed(Resources.this);
					//</editor-fold>
				} catch(InterruptedException ie) {
					//<editor-fold defaultstate="collapsed" desc="Cancelled">
					if (callback != null)
						callback.cancelled(Resources.this);
					//</editor-fold>
				} catch(CancellationException ce) {
					//<editor-fold defaultstate="collapsed" desc="Cancelled">
					if (callback != null)
						callback.cancelled(Resources.this);
					//</editor-fold>
				} catch(RejectedExecutionException ree) {
					//<editor-fold defaultstate="collapsed" desc="Cancelled">
					if (callback != null)
						callback.cancelled(Resources.this);
					//</editor-fold>
				} catch(Throwable t) {
					//<editor-fold defaultstate="collapsed" desc="Error">
					if (callback != null)
						callback.error(Resources.this);
					//</editor-fold>
				}
			}
		});
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Public Static Methods">
	//<editor-fold defaultstate="collapsed" desc="extractAll">
	public static final Future extractAll(final IResourcePackage... packages) {
		final ExecutorService svc = createUnprivilegedExecutorService();
		try {
			return newInstance(packages).extract(packages, svc, IResourceFilter.None, IResourceProgressListener.None, IResourceCallback.None);
		} finally {
			svc.shutdown();
		}
	}

	public static final Future extractAll(final IResourceCallback callback, final IResourcePackage... packages) {
		final ExecutorService svc = createUnprivilegedExecutorService();
		try {
			return newInstance(packages).extract(packages, svc, IResourceFilter.None, IResourceProgressListener.None, callback);
		} finally {
			svc.shutdown();
		}
	}

	public static final Future extractAll(final IResourceProgressListener progress, final IResourceCallback callback, final IResourcePackage... packages) {
		final ExecutorService svc = createUnprivilegedExecutorService();
		try {
			return newInstance(packages).extract(packages, svc, IResourceFilter.None, progress, callback);
		} finally {
			svc.shutdown();
		}
	}

	public static final Future extractAll(final IResourceFilter filter, final IResourceProgressListener progress, final IResourceCallback callback, final IResourcePackage... packages) {
		final ExecutorService svc = createUnprivilegedExecutorService();
		try {
			return newInstance(packages).extract(packages, svc, filter, progress, callback);
		} finally {
			svc.shutdown();
		}
	}

	public static final Future extractAll(final ExecutorService executor, final IResourceFilter filter, final IResourceProgressListener progress, final IResourceCallback callback, final IResourcePackage... packages) {
		return newInstance(packages).extract(packages, executor, filter, progress, callback);
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="newInstance">
	public static final Resources newInstance(final IResourcePackage... Packages) {
		return new Resources(Packages);
	}

	public static final Resources newInstance(final String ResourceName) {
		return newInstance(ResourceProcessorFactory.DEFAULT_INSTANCE, ResourceName);
	}

	public static final Resources newInstance(final File XMLFile) {
		return newInstance(ResourceProcessorFactory.DEFAULT_INSTANCE, XMLFile);
	}

	public static final Resources newInstance(final InputStream XMLData) {
		return newInstance(ResourceProcessorFactory.DEFAULT_INSTANCE, XMLData);
	}

	public static final Resources newInstance(final ResourceProcessorFactory ProcessorFactory, final String ResourceName) {
		return newInstance(ProcessorFactory, Resources.class.getResourceAsStream(ResourceName));
	}

	public static final Resources newInstance(final ResourceProcessorFactory ProcessorFactory, final File XMLFile) {
		FileInputStream fis = null;
		try {
			return newInstance(ProcessorFactory, (fis = new FileInputStream(XMLFile)));
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

	public static final Resources newInstance(final ResourceProcessorFactory ProcessorFactory, final InputStream XMLData) {
		try {
			return new Resources(XMLData);
		} catch(Throwable t) {
			return null;
		}
	}
	//</editor-fold>
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="The Meat">
	protected IResourcePackage[] read(final ResourceProcessorFactory processorFactory, final XPath xpath, final Document document) throws XPathException  {
		Node node;
		NodeList lst;
		IResourcePackage pkg;
		List<IResourcePackage> pkgs = new ArrayList<IResourcePackage>(1);

		//Collapse whitespace nodes
		document.normalize();

		//Get the top-level document element, <Resources />
		final Element top = document.getDocumentElement();

		//Locate <Extract /> tags
		if ((lst = (NodeList)xpath.evaluate("//Resources/Extract", top, XPathConstants.NODESET)) == null || lst.getLength() <= 0)
			return IResourcePackage.EMPTY;

		//Iterate over every <Extract /> tag
		for(int i = 0; i < lst.getLength() && (node = lst.item(i)) != null; ++i) {

			//Ask the package to read it
			if ((pkg = Package.newInstance(processorFactory, node, xpath, document)) != null)
				pkgs.add(pkg);
		}

		//Create an array and return it
		return pkgs.toArray(new IResourcePackage[pkgs.size()]);
	}

	protected void extractResources(final IResourcePackage[] pkgs, final IResourceFilter filter, final IResourceProgressListener progress) throws Exception {
		//Please note that this is executed in a separate thread.
		//It can be cancelled or interrupted at any time.

		int totalPkgs = 0;
		int totalResources = 0;
		long totalBytes = 0;
		String resourceName;
		long endTime;
		boolean success = true;
		Throwable exception = null;
		String title = null;
		long startTime = System.currentTimeMillis();

		try {
			//Notify begin
			notifyProgressBegin(progress, startTime);

			for(IResourcePackage pkg : pkgs) {
				if (pkg == null) {
					notifyProgressReport(progress, totalBytes, totalResources, ++totalPkgs, startTime, "Skipped package");
					continue;
				}

				for(IResourceProcessor p : pkg) {
					if (p == null) {
						notifyProgressReport(progress, totalBytes, ++totalResources, totalPkgs, startTime, "Skipped resource");
						continue;
					}

					//Double check that we're allowed to process this resource
					resourceName = pkg.resourcePath(p.getName());
					if (filter != null && !filter.filter(pkg, p, resourceName)) {
						notifyProgressReport(progress, totalBytes, ++totalResources, totalPkgs, startTime, "Skipped resource");
						continue;
					}

					//Here we go!
					if (p.process(resourceName, pkg, progress)) {
						totalBytes += p.getSize();
						title = (p instanceof DefaultResourceProcessor ? ((DefaultResourceProcessor)p).getTitle() : p.getName());
						if (StringUtil.isNullOrEmpty(title))
							title = p.getName();
						notifyProgressReport(progress, totalBytes, ++totalResources, totalPkgs, startTime, title);
					}
				}

				notifyProgressReport(progress, totalBytes, totalResources, ++totalPkgs, startTime, "Completed package");
			}
			success = true;
		} catch(Throwable t) {
			success = false;
			exception = t;
		} finally {
			endTime = System.currentTimeMillis();
		}

		if (exception != null)
			notifyProgressError(progress, exception, !StringUtil.isNullOrEmpty(exception.getMessage()) ? exception.getMessage() : "Error loading resources");
		notifyProgressEnd(progress, success, totalBytes, totalResources, totalPkgs, startTime, endTime);
	}
	//</editor-fold>
}
