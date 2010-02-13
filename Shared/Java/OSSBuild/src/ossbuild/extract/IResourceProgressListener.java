
package ossbuild.extract;

/**
 * Allows interested parties to be notified of resource extraction progress.
 * 
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public interface IResourceProgressListener {
	//<editor-fold defaultstate="collapsed" desc="Constants">
	public static final IResourceProgressListener None = null;
	//</editor-fold>

	void begin(final int totalNumberOfResources, final int totalNumberOfPackages, final long totalNumberOfBytes, final long startTime);
	void report(final int totalNumberOfResources, final int totalNumberOfPackages, final long totalNumberOfBytes, final long numberOfBytesCompleted, final int numberOfResourcesCompleted, final int numberOfPackagesCompleted, final long startTime, final long duration, final String message);
	void error(final Throwable exception, final String message);
	void end(final boolean success, final int totalNumberOfResources, final int totalNumberOfPackages, final long totalNumberOfBytes, final long numberOfBytesCompleted, final int numberOfResourcesCompleted, final int numberOfPackagesCompleted, final long startTime, final long endTime);

}
