#pragma once
#include <fstream>

// TODO: Moemory pooling

namespace srl
{
	template<typename T>
	class LinkedList
	{
	public:
		class Node;
	public:
		LinkedList()
			: mRoot(nullptr)
		{
		}

		~LinkedList()
		{
			Free();
		}

		void Free()
		{
			if (mRoot == nullptr)
			{
				return;
			}
			Node* currNode = mRoot;
			while (currNode->Next != nullptr)
			{
				Node* prevNode = currNode;
				currNode = currNode->Next;
				delete prevNode;
			}
			mRoot = nullptr;
		}

		void Append(const T& data)
		{
			Node* newNode = nullptr;
			if (mRoot == nullptr)
			{
				mRoot = new Node;
				newNode = mRoot;
			}
			else
			{
				Node* tail = Tail();
				tail->Next = new Node;
				newNode = tail->Next;
			}
			newNode->Data = data;
		}

		void Delete(const Node& node)
		{
			if (mRoot == nullptr)
			{
				return;
			}
			Node* currNode = mRoot;
			Node* prevNode = nullptr;
			while (currNode != nullptr)
			{
				if (currNode == &node)
				{
					if (prevNode == nullptr)
					{
						delete currNode;
						mRoot = nullptr;
					}
					else
					{
						prevNode.Next = currNode.Next;
						delete currNode;
					}
					break;
				}
				currNode = currNode->Next;
			}
		}

		Node* operator[](size_t idx)
		{
			if (mRoot == nullptr)
			{
				return nullptr;
			}

			size_t i = 0;
			Node* currNode = mRoot;
			while (currNode != nullptr && i < idx)
			{
				currNode = currNode->Next;
				i++;
			}
			return currNode;
		}

		Node* Tail()
		{
			Node* currNode = mRoot;
			while (currNode->Next != nullptr)
			{
				continue;
			}
			return currNode;
		}

		size_t Size()
		{
			if (mRoot == nullptr)
			{
				return 0;
			}

			size_t size = 0;
			Node* currNode = mRoot;
			while (currNode != nullptr)
			{
				currNode = currNode->Next;
				size++;
			}
			return size;
		}

		void Write(std::ofstream& fout)
		{
			// Write label list
			size_t size = Size();
			fout.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
			auto currNode = mRoot;
			while (currNode != nullptr)
			{
				T& data = currNode->Data;
				fout.write(reinterpret_cast<const char*>(&data), sizeof(T));
				currNode = currNode->Next;
			}
		}

		void Read(std::ifstream& fin)
		{
			size_t size = 0;
			fin.read(reinterpret_cast<char*>(&size), sizeof(size_t));
			for (size_t i = 0; i < size; i++)
			{
				T data;
				fin.read(reinterpret_cast<char*>(&data), sizeof(T));
				Append(data);
			}
		}

	public:
		struct Node
		{
			T Data;
			Node* Next = nullptr;
		};
		Node* mRoot;
	};
}