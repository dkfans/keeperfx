package keeperfx.configtool;

public class KeeperFXConfiguration extends ConfigurationBuffer {
	public KeeperFXConfiguration() {
		super("(.+)=(.*)", "ISO-8859-1");
	}

	@Override
	protected void newItem(String key, String value) {
		data.append(key);
		data.append('=');
		data.append(value);
		data.append('\n');
	}
}
