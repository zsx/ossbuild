
package ossbuild.extract;

/**
 * Exception regarding resource extraction and/or loading.
 * 
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public class ResourceUnavailableException extends RuntimeException {
    public ResourceUnavailableException() {
    }

    public ResourceUnavailableException(String message) {
        super(message);
    }

    public ResourceUnavailableException(Throwable cause) {
        super(cause);
    }

    public ResourceUnavailableException(String message, Throwable cause) {
        super(message, cause);
    }
}
