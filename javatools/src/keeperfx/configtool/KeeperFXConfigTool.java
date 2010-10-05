package keeperfx.configtool;

import javax.swing.SwingUtilities;

public class KeeperFXConfigTool {
	public static void main(String[] args) {
		SwingUtilities.invokeLater(new Runnable() {
			@Override
			public void run() {
				new MainWindow().setVisible(true);
			}
		});
	}
}
