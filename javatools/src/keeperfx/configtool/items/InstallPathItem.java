package keeperfx.configtool.items;

import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JTextField;

import keeperfx.configtool.ConfigurationBuffer;
import keeperfx.util.ValueObservable;

public class InstallPathItem extends ConfigurationItem implements ActionListener {
	private static final long serialVersionUID = -6534297257031185951L;
	private final JTextField field;

	public InstallPathItem(ValueObservable<Boolean> configChanged) {
		super(configChanged, "Installation Path:");
		
		field = new JTextField();
		field.setPreferredSize(new Dimension(400, 20));
		field.addActionListener(this);
		add(field);
	}

	@Override
	public void load(ConfigurationBuffer config) {
		field.setText(config.getItem("INSTALL_PATH", "./"));
	}

	@Override
	public void save(ConfigurationBuffer config) {
		config.setItem("INSTALL_PATH", field.getText());
	}

	@Override
	public void actionPerformed(ActionEvent ev) {
		markChange();
	}
}
