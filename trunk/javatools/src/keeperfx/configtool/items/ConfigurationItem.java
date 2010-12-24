package keeperfx.configtool.items;

import javax.swing.BorderFactory;
import javax.swing.JPanel;

import keeperfx.configtool.ConfigurationBuffer;
import keeperfx.util.ValueObservable;

public abstract class ConfigurationItem extends JPanel {
	private static final long serialVersionUID = -5970682790739020135L;
	private final ValueObservable<Boolean> configChanged;
	
	public ConfigurationItem(ValueObservable<Boolean> configChanged, String label) {
		super();
		this.configChanged = configChanged;
		setBorder(BorderFactory.createTitledBorder(label));
	}
	
	public abstract void load(ConfigurationBuffer config);
	public abstract void save(ConfigurationBuffer config);
	
	protected void markChange() {
		configChanged.setValue(true);
	}
}
