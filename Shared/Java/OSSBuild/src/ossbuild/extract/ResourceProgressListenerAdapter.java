
package ossbuild.extract;

/**
 * Adapter for {@link IResourceProgressListener}.
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public abstract class ResourceProgressListenerAdapter implements IResourceProgressListener {

	public void begin(int totalNumberOfResources, int totalNumberOfPackages, long totalNumberOfBytes, long startTime) {
	}

	public void report(int totalNumberOfResources, int totalNumberOfPackages, long totalNumberOfBytes, long numberOfBytesCompleted, int numberOfResourcesCompleted, int numberOfPackagesCompleted, long startTime, long duration, String message) {
	}

	public void error(Throwable exception, String message) {
	}

	public void end(boolean success, int totalNumberOfResources, int totalNumberOfPackages, long totalNumberOfBytes, long numberOfBytesCompleted, int numberOfResourcesCompleted, int numberOfPackagesCompleted, long startTime, long endTime) {
	}

}
