#pragma once

template <typename T>
class List {
public:
	class Node {
	public:
		Node(T &elem) : m_elem(elem), m_pNext(NULL) { }
		T &Get() const {
			return &m_elem;
		}
		Node *Next() {
			return m_pNext;
		}
	protected:
		friend List;

		T m_elem;
		Node *m_pNext;
	};

	List(): m_pFirst(NULL) { }
	void Add(T &elem) {
		if (m_pFirst == NULL) {
			m_pLast = m_pFirst = new Node(elem);
		} else {
			m_pLast->m_pNext = new Node(elem);
		}
	}
	Node *Begin() { return m_pFirst; }
	Node *End() { return m_pLast; }
protected:
	Node *m_pFirst;
	Node *m_pLast;
};