/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package ossbuild.extract;

/**
 * Accepts or rejects a library or file being loaded/extracted.
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public interface IFilter {
	boolean filterForExtraction(String name);
	boolean filterForLibraryLoad(String name);
}
