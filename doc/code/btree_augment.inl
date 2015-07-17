struct average_augment {
	average_augment() : sum(0), count(0) {}

	int sum;
	int count;
};

struct average_augmenter {
	template <typename N>
	average_augmenter operator()(const N & node) {
		if (node.is_leaf()) {
			average_augment answer;

			for(size_t i = 0; i < node.count(); ++i) {
				average_augment answer;
				answer.sum += node.value(i);
				answer.count++;
			}

			return answer;
		} 
		
		average_augment answer;
		for(size_t i = 0; i < node.count(); ++i) {
			average_augment augment = node.get_augmentation(i);
			answer.sum += augment.sum;
			answer.count += augment.count;
		}

		return answer;
	}
};