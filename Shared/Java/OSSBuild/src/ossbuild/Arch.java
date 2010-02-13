
package ossbuild;

/**
 * Gathers information about operating systems and the one we're hosted on.
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public enum Arch {
	   Unknown(
		  ArchWordSize.Unknown
		, StringUtil.empty
	), x86(
		  ArchWordSize.Size32Bits
		, "x86"

		, "x86"
		, "i386"
		, "i686"
	), x86_64(
		ArchWordSize.Size64Bits
		, "x86_64"

		, "x86_64"
		, "x64"
		, "amd64"
	), IA64(
		ArchWordSize.Size64Bits
		, "ia64"

		, "IA64N"
	), PPC(
		ArchWordSize.Size32Bits
		, "ppc"

		, "ppc"
		, "PowerPC"
		, "Power"
	), PPC64(
		ArchWordSize.Size64Bits
		, "ppc64"

		, "ppc64"
	), Arm(
		ArchWordSize.Size32Bits
		, "arm"

		, "arm"
	), ArmV4I(
		ArchWordSize.Size32Bits
		, "armv4i"

		, "armv4i"
	), Sparc(
		ArchWordSize.Size64Bits
		, "sparc"

		, "sparc"
	), PA_RISC(
		ArchWordSize.Size64Bits
		, "pa_risc"

		, "PA-RISC"
		, "PA_RISC2.0"
	), POWER_RS(
		ArchWordSize.Unknown
		, "power_rs"

		, "POWER_RS"
	), MIPS(
		ArchWordSize.Size64Bits
		, "mips"

		, "mips"
	), Alpha(
		ArchWordSize.Size64Bits
		, "alpha"
		
		, "alpha"
	)
	;

	//<editor-fold defaultstate="collapsed" desc="Constants">
	public static final String
		NAME = System.getProperty("os.arch")
	;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Variables">
	private static Arch systemArch;
	private String[] variants;
	private ArchWordSize wordSize;
	private String platformPartName;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Initialization">
	static {
		systemArch = fromName(NAME);
	}

	Arch(final ArchWordSize WordSize, final String PlatformPartName, final String...Variations) {
		this.variants = Variations;
		this.wordSize = WordSize;
		this.platformPartName = PlatformPartName;
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Getters">
	public static String getSystemArchName() {
		return NAME;
	}

	public static Arch getSystemArch() {
		return systemArch;
	}

	public String[] getVariants() {
		return variants;
	}

	public ArchWordSize getWordSize() {
		return wordSize;
	}

	public String getPlatformPartName() {
		return platformPartName;
	}

	public boolean is8Bit() {
		return wordSize == ArchWordSize.Size8Bits;
	}

	public boolean is16Bit() {
		return wordSize == ArchWordSize.Size16Bits;
	}

	public boolean is32Bit() {
		return wordSize == ArchWordSize.Size32Bits;
	}

	public boolean is64Bit() {
		return wordSize == ArchWordSize.Size64Bits;
	}

	public boolean is128Bit() {
		return wordSize == ArchWordSize.Size128Bits;
	}

	public boolean is256Bit() {
		return wordSize == ArchWordSize.Size256Bits;
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Public Static Methods">
	public static Arch fromName(final String Name) {
		if (StringUtil.isNullOrEmpty(Name))
			return Arch.Unknown;

		for(Arch a : Arch.values()) {
			for(String variant : a.variants)
				if (variant.equalsIgnoreCase(Name))
					return a;
		}

		return Arch.Unknown;
	}
	//</editor-fold>
}
