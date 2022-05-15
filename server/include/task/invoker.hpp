#pragma once

#include "task.hpp"

/**
 * @interface IInvoker нужен для выполнения указанной задачи
 */
class IInvoker
{
public:
	/**
	 * Выполнить указанную задачу
	 * @param task Задача
	 * @return true задача поставлена в очередь, false - достигнут максимальный
	 * лимит задач
	 */
	virtual bool invoke(TaskPtr task) = 0;

	virtual ~IInvoker() = default;
};