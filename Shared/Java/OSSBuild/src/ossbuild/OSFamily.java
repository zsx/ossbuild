
package ossbuild;

/**
 * Gathers information about operating systems and the one we're hosted on.
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public enum OSFamily {
	  Unknown (StringUtil.empty)

	, Windows ("windows")
	, Mac     ("osx")
	, Unix    ("unix")
	, Solaris ("solaris")
	, VMS     ("vms")
	
	;

	//<editor-fold defaultstate="collapsed" desc="Variables">
	private String platformPartName;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Initialization">
	OSFamily(final String PlatformPartName) {
		this.platformPartName = PlatformPartName;
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Getters">
	public static OSFamily getSystemOSFamily() {
		return OS.getSystemOSFamily();
	}

	public String getPlatformPartName() {
		return platformPartName;
	}

	public boolean isPOSIX() {
		return isPOSIX(this);
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Public Static Methods">
	public static boolean isPOSIX(final OS OS) {
		return isPOSIX(OS.getFamily());
	}

	public static boolean isPOSIX(final OSFamily OSFamily) {
		switch(OSFamily) {
			case Unix:
			case Mac:
			case Solaris:
				return true;
			default:
				return false;
		}
	}

	public static OSFamily fromName(final String Name) {
		if (StringUtil.isNullOrEmpty(Name))
			return OSFamily.Unknown;

		for(OSFamily family : OSFamily.values()) {
			if (family.platformPartName.equalsIgnoreCase(Name))
				return family;
		}

		final String lower = Name.toLowerCase();
		if (lower.contains("win"))
			return OSFamily.Windows;
		else if (lower.contains("mac"))
			return OSFamily.Mac;
		else if (lower.contains("nix") || lower.contains("nux"))
			return OSFamily.Unix;
		else if (lower.contains("vms"))
			return OSFamily.VMS;
		else if (lower.contains("solaris"))
			return OSFamily.Solaris;
		else
			return OSFamily.Unknown;
	}
	//</editor-fold>
}
