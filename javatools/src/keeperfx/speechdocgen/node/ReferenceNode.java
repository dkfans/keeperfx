package keeperfx.speechdocgen.node;

import java.util.ArrayList;
import java.util.HashMap;

public class ReferenceNode extends ArrayNode {
	private static final long serialVersionUID = 7616148403118053800L;
	private final String refid;

	public ReferenceNode(String refid) {
		this.refid = refid;
	}

	@Override
	public void resolveReferences(HashMap<String, ArrayNode> targets,
			ArrayList<ReferenceNode> refs) {
		refs.add(this);
	}

	public void resolve(HashMap<String, ArrayNode> targets) {
		ArrayNode target = targets.get(refid);
		if (target != null) {
			int index = getParent().indexOf(this);
			getParent().remove(index);
			
			for (int i = target.size() - 1; i >= 0; --i) {
				getParent().add(index, target.get(i));
			}
		}
	}
}
