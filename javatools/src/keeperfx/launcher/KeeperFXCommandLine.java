package keeperfx.launcher;

import keeperfx.configtool.ConfigurationBuffer;

public class KeeperFXCommandLine extends ConfigurationBuffer {
	public KeeperFXCommandLine() {
		super("(-\\S*)(\\S&&[^-]+)?", "US-ASCII");
	}

	@Override
	protected void newItem(String key, String value) {
		if (data.length() > 0 && data.charAt(data.length() - 1) != ' ') {
			data.append(' ');
		}
		
		data.append(key);
		if (value != null) {
			data.append('=');
			data.append(value);
		}
	}

	@Override
	public void clearItem(String key) {
		super.clearItem('-' + key);
	}

	@Override
	public String getItem(String key) {
		return super.getItem('-' + key);
	}

	@Override
	public boolean hasItem(String key) {
		return super.hasItem('-' + key);
	}

	@Override
	public void setItem(String key, String value) {
		super.setItem('-' + key, value);
	}
}
